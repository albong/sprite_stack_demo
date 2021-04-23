#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include "graphics.h"
#include "text.h"

/*
 * You can't have a master font, you really need a font for each language.
 * 
 * For a given font (recommend the noto fonts in a heavier variety), use BMFont or fontbm (see below)
 * to generate image files of the font, along with a binary data file that we can load here.
 * 
 * For each font that you want to use, you'll need to create this and change the fonts over when
 * you change a language.
 * 
 * Presently we are using 32-pixel Noto Sans Light for english (I plan to change to Sans SemiBold),
 * use this as a reference for the size of other fonts.
 * -------
 * this is based on the output of BMFont, from: http://www.angelcode.com/products/bmfont/
 * an apparently compatible linux alternative to BMFont is fontbm: https://github.com/vladimirgamalyan/fontbm
 */


/////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////
//NOTE: These are like, packed, I guess?  If my arithmetic is right it doesn't really matter.  I tried for memory savings
//however, we probably could reduce all of the members of FontCharacter by a byte, since our font is so small
typedef struct FontCharacter {
    uint32_t id;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    int16_t xOffset;
    int16_t yOffset;
    int16_t xAdvance;
    uint8_t sheetNum;
} FontCharacter;

typedef struct Font {
    char *name;
    Image **fontSheets; //array
    ImageAtlas *fontSheetAtlas;
    FontCharacter *characters; //array
    uint32_t numCharacters;
    uint16_t numSheets;
    uint16_t baseHeight;
    uint16_t lineHeight;
} Font;


/////////////////////////////////////////////////
// Loading/Unloading
/////////////////////////////////////////////////
void initFonts();
void termFonts();
void free_Font(Font *self);


/////////////////////////////////////////////////
// Drawing
/////////////////////////////////////////////////
/*
 * Returns how much the cursor has advanced from where it started
 */
int drawText(Text *text, Font *font, int textStart, int numCharacters, int cursorX, int cursorY);
int drawCharacter(uint32_t id, Font *font, int cursorX, int cursorY);


/////////////////////////////////////////////////
// Access
/////////////////////////////////////////////////
Font *getCurrentTextFont();
int getWidthOfText(Font *font, Text *text, int startIndex, int length);
FontCharacter *findCharacter(Font *font, uint32_t id); //valid to return null


#endif
