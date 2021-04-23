#include "font.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_reader.h"
#include "graphics.h"
#include "constants.h"
#include "logging.h"
#include "omni_exit.h"
#include "text.h"

// I expect to be reading a binary file, in the correct format, as specified on the BMFont site
//
// if you're wondering why I didn't just read into structs and such, its because idgaf
//
// I think the computer I generated the original fnt file on is little endian, if you generate
// a big endian file this is all boned maybe, or maybe not, since using SDL reading should'ved fixed that

static Font *loadFont(char *fontName);
static void readCommonBlock(uint8_t *block, uint32_t blockSize, Font *font);
static void readPagesBlock(uint8_t *block, uint32_t blockSize, Font *font);
static void readCharsBlock(uint8_t *block, uint32_t blockSize, Font *font);
static uint16_t readUInt16FromBlock(uint8_t *block);
static uint16_t readInt16FromBlock(uint8_t *block);
static uint32_t readUInt32FromBlock(uint8_t *block);
static int compareFontCharacters(const void *a, const void *b);

//we'd like to add another, larger, font here for the menu
static Font *currentTextFont = NULL;
static const int TOFU_WIDTH = 16; //for drawing missing characters

/////////////////////////////////////////////////
// Loading/Unloading
/////////////////////////////////////////////////
void initFonts(){
    //we may call this function again later to reload fonts on language change
    if (currentTextFont != NULL){
        termFonts();
    }
    
    LanguageCode currentLC = getCurrentLanguage();
    if (currentLC == JP_LC){
        //jp uses different font than other languages because they have different versions of characters as compared to chinese and korean (not that we use korean hanja)
        LOG_ERR("JP font not yet supported!");
        //not sure if we should crash here or respond correctly
        currentTextFont = NULL;
    } else {
        //currentTextFont = loadFontForLanguage("en");
        currentTextFont = loadFont("sans");
    }
    
    //setTextboxFont(currentTextFont);
}

void termFonts(){
    free_Font(currentTextFont);
    currentTextFont = NULL;
}

void free_Font(Font *self){
    if (self == NULL){
        LOG_WAR("Tried to free NULL font");
        return;
    }
    
    free(self->name);
    
    int i;
    for (i = 0; i < self->numSheets; i++){
        free_Image(self->fontSheets[i]);
        self->fontSheets[i] = NULL;
    }
    free(self->fontSheets);
    free_ImageAtlas(self->fontSheetAtlas);
    
    free(self->characters);
    
    self->numCharacters = 0;
    self->numSheets = 0;
    self->baseHeight = 0;
    self->lineHeight = 0;
    
    free(self);
}


/////////////////////////////////////////////////
// Drawing
/////////////////////////////////////////////////
int drawText(Text *text, Font *font, int textStart, int numCharacters, int cursorX, int cursorY){
    int i, j;
    int originalCursorX;
    
    int textEnd = textStart + numCharacters;
    if (textEnd > text->length){
        textEnd = text->length;
    }
    
    for (i = textStart; i < textEnd; i++){
        cursorX += drawCharacter(text->ids[i], font, cursorX, cursorY);
    }
    
    return cursorX - originalCursorX;
}

int drawCharacter(uint32_t id, Font *font, int cursorX, int cursorY){
    /*
     * How to make sense for drawing:
     * (see http://www.angelcode.com/products/bmfont/doc/render_text.html)
     * 
     *  - lineHeight is how tall the lines should be
     *  - lines need a little padding on the end
     *  - given the id of a character to draw, find its FontCharacter;
     *     x, y, width, and height correspond to the rectangle about the
     *     character in the sheet given by sheetNum
     *     you draw that rectangle where your "cursor" is, offset by the
     *     given offsets (which may be negative)
     *     finally, you advance your "cursor" by advanceX, which is where
     *     you'll draw the next character
     *  - baseHeight is the height from the top of the line to the bottom
     *     of where letters are drawn, which in hindsight is basically useless
     */
    FontCharacter *fc;
    ImageRect src, dst;
    int advance;
    
    fc = findCharacter(font, id);
    if (fc != NULL){
        //where is the character in the font sheet
        src.x = fc->x;
        src.y = fc->y;
        src.w = fc->width;
        src.h = fc->height;
        
        //adjust the destination accordingly
        dst.x = cursorX + fc->xOffset;
        dst.y = cursorY + fc->yOffset;
        dst.w = fc->width;
        dst.h = fc->height;
        
        //draw
        drawImageSrcDst(font->fontSheets[fc->sheetNum], &src, &dst);
        
        //advance the cursor
        advance = fc->xAdvance;
        
    //if the character can't be found, draw a red tofu
    } else {
        drawFilledRect(cursorX, cursorY, TOFU_WIDTH, font->lineHeight, 255, 0, 0);
        advance = TOFU_WIDTH;
    }
    
    return advance;
}


/////////////////////////////////////////////////
// Access
/////////////////////////////////////////////////
Font *getCurrentTextFont(){
    return currentTextFont;
}

int getWidthOfText(Font *font, Text *text, int startIndex, int length){
    size_t i;
    int result = 0;
    FontCharacter *c;
    
    //null check
    if (startIndex >= text->length){
        result = 0;
    } else {    
        //add up the lengths
        for (i = startIndex; (i - startIndex) < length && i < text->length; i++){
            c = findCharacter(font, text->ids[i]);
            if (c != NULL){
                //take the max of the sizes
                // if (c->xAdvance > c->width + c->xOffset){
                    result += c->xAdvance;
                // } else {
                    // result += c->width + c->xOffset;
                // }
            }
        }
    }
    
    return result;
}

FontCharacter *findCharacter(Font *font, uint32_t id){
    FontCharacter *result = NULL; //valid to return NULL
    FontCharacter key;
    key.id = id;
    result = bsearch(&key, font->characters, font->numCharacters, sizeof(FontCharacter), compareFontCharacters);
    
    return result;
}


/////////////////////////////////////////////////
// Font file reading
/////////////////////////////////////////////////
Font *loadFont(char *fontName){
    char filename[80];
    uint8_t *data;
    size_t index;
    char blockType;
    uint32_t blockSize;
    uint8_t *block;
    unsigned long fileSize;
        
    //read in the file
    sprintf(filename, "data/fonts/%s.fnt", fontName);
    data = readBinaryFileToCharStar(filename, &fileSize);  //PIZZA type mismatch possible here
    
    //size check, header check
    //this magic number is the bare minimum number of bytes to produce a valid Font struct
    if (fileSize < (4 + 5*4 + 15 + 15 + 1 + 20) || (data[0] != 66 && data[0] != 77 && data[0] != 70 && data[0] != 3)){ 
        LOG_ERR("Problem with font file %s", filename);
        displayErrorAndExit("Problem reading font file");
    }
    
    Font *font = malloc(sizeof(Font));
    font->name = strdup(fontName);
    
    //loop over all the data
    index = 4;
    while (index < fileSize){
        //read int the block header
        blockType = data[index];
        blockSize = readUInt32FromBlock(data + index + 1);
        
        //"copy" the block
        block = data + index + 5;//five bytes for the header
        
        //handle the block
        switch (blockType) {
            case 2:
                readCommonBlock(block, blockSize, font);
                break;
            case 3:
                readPagesBlock(block, blockSize, font);
                break;
            case 4:
                readCharsBlock(block, blockSize, font);
                break;
            default:
                //case 1 would be the info block, which has nothing of value for us
                //case 5 would be kerning pairs, which we ignore
                break;
        }
        
        //increment
        index += blockSize + 5; //five bytes for the header
    }
    
    free(data);
    return font;
}

void readCommonBlock(uint8_t *block, uint32_t blockSize, Font *font){
    size_t index = 0;
    
    //first two bytes are line height
    font->lineHeight = readUInt16FromBlock(block + index);
    index += 2;
    
    //next two bytes are the baseHeight
    font->baseHeight = readUInt16FromBlock(block + index);
    index += 2;
    
    //skip ahead 4 bytes to the number of tilesheets (pages in the naming of BMFont)
    index += 4;
    font->numSheets = readUInt16FromBlock(block + index);
    
    //I don't care about anything else in this block, but there is more
}

void readPagesBlock(uint8_t *block, uint32_t blockSize, Font *font){
    //I guess in theory, the blocks might be out of order?  in which case we might not know how many tilesheets there are at this point
    //Its unclear, and maybe we ought to handle it?
    //
    //Since the filenames are null separated, can just pass the whole block as a string
    size_t i;
    char filename[FILENAME_BUFFER_SIZE];
    
    //check if the sizes are the same
    if (blockSize / (strlen(block)+1) != font->numSheets){
        LOG_ERR("Font file corrupted OR font file blocks are out of order");
        displayErrorAndExit("Problem reading font file");
    }
    
    //allocate the array
    font->fontSheets = malloc(sizeof(Image *) * font->numSheets);
    
    //go over each sheet and load the image
    startBatchingLoadedImages();
    for (i = 0; i < font->numSheets; i++){
        sprintf(filename, "gfx/fonts/%s/%s", font->name, block);
        font->fontSheets[i] = loadImageFromFile(filename);
        block += strlen(block)+1;
    }
    font->fontSheetAtlas = stopBatchingLoadedImages();
}

void readCharsBlock(uint8_t *block, uint32_t blockSize, Font *font){
    //i realize now these ought have switched names
    size_t index = 0;
    uint32_t position = 0;
    
    //get the number of characters and allocate
    font->numCharacters = blockSize / 20; //each field is 20 bytes
    font->characters = malloc(sizeof(FontCharacter) * font->numCharacters);
    
    //loop over the block
    while (index < blockSize && position < font->numCharacters){
        //first four bytes are an id
        font->characters[position].id = readUInt32FromBlock(block + index);
        index += 4;
        
        //next two bytes are x, and the two after are y
        font->characters[position].x = readUInt16FromBlock(block + index);
        index += 2;
        font->characters[position].y = readUInt16FromBlock(block + index);
        index += 2;
        
        //the next two pairs are width and height
        font->characters[position].width = readUInt16FromBlock(block + index);
        index += 2;
        font->characters[position].height = readUInt16FromBlock(block + index);
        index += 2;
        
        //then xoffset and yoffset
        font->characters[position].xOffset = readInt16FromBlock(block + index);
        index += 2;
        font->characters[position].yOffset = readInt16FromBlock(block + index);
        index += 2;
        
        //xadvance is 2 bytes
        font->characters[position].xAdvance = readInt16FromBlock(block + index);
        index += 2;
        
        //the page (tilesheet number) is only 1 byte
        font->characters[position].sheetNum = block[index];
        index += 1;
        
        //and we don't care about the channel
        index +=1;
        
        //increment
        position++;
    }
    
    //sort to make lookup times faster ON AVERAGE
    qsort(font->characters, font->numCharacters, sizeof(FontCharacter), compareFontCharacters);
}

uint16_t readUInt16FromBlock(uint8_t *block){
    return ((uint16_t)block[0+1]<<8) | ((uint16_t)block[0]);
}

uint16_t readInt16FromBlock(uint8_t *block){
    return (int16_t)(((uint16_t)block[0+1]<<8) | ((uint16_t)block[0]));
}

uint32_t readUInt32FromBlock(uint8_t *block){
    return ((uint32_t)block[0+3]<<24) | ((uint32_t)block[0+2]<<16) | ((uint32_t)block[0+1]<<8) | ((uint32_t)block[0]);
}

int compareFontCharacters(const void *a, const void *b){
    FontCharacter *aCharacter = (FontCharacter *)a;
    FontCharacter *bCharacter = (FontCharacter *)b;
    
    //ids are unsigned, and implicit casts are bad news bears
    if (aCharacter->id < bCharacter->id){
        return -1;
    } else if (aCharacter->id > bCharacter->id){
        return 1;
    } else { 
        return 0;
    }
}
