#include "text.h"
#include "logging.h"
#include "omni_exit.h"
#include "constants.h"
#include "file_reader.h"
#include "../lib/cjson_wrapper.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

static LanguageCode currentLanguageCode = EN_LC;
static Text *textTable;
static size_t textTableLength;

static void readByteRangeToText(unsigned char *data, int startByte, int endByte, Text *result); //crashes on error


/////////////////////////////////////////////////
// Loading/Unloading
/////////////////////////////////////////////////
void termText(){
    size_t i;
    for (i = 0; i < textTableLength; i++){
        if (textTable[i].length != 0){
            free(textTable[i].ids);
        }
    }
    free(textTable);
    textTableLength = 0;
}

Text *init_Text(Text *self){
    self->length = 0;
    self->ids = NULL;
    
    return self;
}

void free_Text(Text *self){
    if (self == NULL){
        LOG_WAR("Tried to free NULL text");
        return;
    }
    
    free(self->ids);
    free(self);
}

void loadTextForCurrentLanguage(){
    int errorCount = 0;
    char filename[FILENAME_BUFFER_SIZE];
    char manifestFilename[FILENAME_BUFFER_SIZE];
    
    //choose the filname based on the lanuage
    switch (currentLanguageCode){
        case EN_LC:
            sprintf(filename, "data/text/text.en");
            sprintf(manifestFilename, "data/text/text.en.manifest");
            break;
        
        case KO_LC:
            sprintf(filename, "data/text/text.ko");
            sprintf(manifestFilename, "data/text/text.ko.manifest");
            break;
        
        default:
            LOG_ERR("Language code not set!");
            filename[0]='\0';
            manifestFilename[0]='\0';
            break;
    }
    
    //read in the text file
    unsigned long fileLength;
    unsigned char *data = readBinaryFileToCharStar(filename, &fileLength);
    
    //read in the manifest
    char *manifestContents = readFileToString(manifestFilename);
    cJSON *root = cJSON_Parse(manifestContents);
    if (!root){
        LOG_ERR("Failed to parse %s:\n%s", manifestFilename, cJSON_GetErrorPtr());
        displayErrorAndExit("Problem reading data file");
    }
    
    //pull out relevant arrays
    cJSON *jsonIds = cjson_readArray(root, "ids", &errorCount);
    cJSON *jsonStartByte = cjson_readArray(root, "start byte", &errorCount);
    cJSON *jsonEndByte = cjson_readArray(root, "end byte", &errorCount);
    
    //sanity check
    int textCount = cjson_getArrayLength(jsonIds, &errorCount);
    int startByteCount = cjson_getArrayLength(jsonStartByte, &errorCount);
    int endByteCount = cjson_getArrayLength(jsonEndByte, &errorCount);
    if (startByteCount != textCount || endByteCount != textCount || startByteCount != endByteCount){
        LOG_ERR("Arrays in the text json file do not match!");
        displayErrorAndExit("Data file corruption detected");
    }
    
    //determine how large to make the text array
    int largestId = -1;
    int i;
    for (i = 0; i < textCount; i++){
        if (cJSON_GetArrayItem(jsonIds, i)->valueint > largestId){
            largestId = cjson_readIntFromArray(jsonIds, i, &errorCount);
        }
    }
    Text *textById = calloc(largestId + 1, sizeof(Text)); //+1 since zero indexed
    
    //load text
    int id, startByte, endByte;
    for (i = 0; i < textCount; i++){
        id = cJSON_GetArrayItem(jsonIds, i)->valueint;
        startByte = cjson_readIntFromArray(jsonStartByte, i, &errorCount);
        endByte = cjson_readIntFromArray(jsonEndByte, i, &errorCount);
        
        readByteRangeToText(data, startByte, endByte, textById + id);
    }
    
    //free everything
    free(data);
    cJSON_Delete(root);
    free(manifestContents);
    
    //set the variables
    textTable = textById;
    textTableLength = largestId + 1;
}


/////////////////////////////////////////////////
// Access
/////////////////////////////////////////////////
void setCurrentLanguage(LanguageCode code){
    currentLanguageCode = code;
}

LanguageCode getCurrentLanguage(){
    return currentLanguageCode;
}

const char *getCurrentLanguageString(){
    char *result;
    
    switch (currentLanguageCode){
        case EN_LC:
            return "en";
            break;
        
        case KO_LC:
            return "ko";
            break;
        
        default:
            return "";
            break;
    }
}

void readByteRangeToText(unsigned char *data, int startByte, int endByte, Text *result){
    /*
     * Here's the thing.
     * The startByte and endByte come from a json file.  cJSON only reads ints, not longs or size_t's.
     * Conclusion: if the text file is very large, we will have a bug.
     */
    
    /*
    * UTF8 CONVERSION START
    */
    uint32_t *codes = NULL;
    size_t i;
    size_t numChars = 0;
    size_t expectedLength = 0;
    size_t currChar = 0;
    
    //first, count the number of characters
    for (i = startByte; i <= endByte && data[i] != '\0'; i++){
        //check if the leading bits match what utf8 wants for the first byte
        //0xxxxxxx, 110xxxxx, 1110xxxx, 11110xxx
        //could of course replace with more bitwise operators over logical operators, but probably good enough
        //0xxxxxxx
        if (((~data[i]) & 128) == 128){
            numChars++;
            expectedLength += 1;

        //110xxxxx
        } else if ((data[i] & 192) == 192 && ((~data[i]) & 32) == 32) {
            numChars++;
            expectedLength += 2;

        //1110xxxx
        } else if ((data[i] & 224) == 224 && ((~data[i]) & 16) == 16) {
            numChars++;
            expectedLength += 3;

        //11110xxx
        } else if ((data[i] & 240) == 240 && ((~data[i]) & 8) == 8) {
            numChars++;
            expectedLength += 4;
        }
    }

    //if the expected number of bytes is incorrect, fail
    if (expectedLength != (endByte - startByte + 1)){
        LOG_ERR("Expected UTF8 length does not match actual length; expected %zu bytes but had %d", expectedLength, (endByte - startByte + 1));
        displayErrorAndExit("Error encountered reading data file");
    }
        
    //allocate an array for the codes
    codes = calloc(numChars, sizeof(uint32_t));
    
    //reread the data and convert to utf8 code points in decimal
    currChar = 0;
    i = startByte;
    while (i <= endByte && data[i] != '\0'){
        //as before, could use bitwise operators over logical ones, but don't see need for such optimization yet
        //0xxxxxxx
        if (((~data[i]) & 128) == 128){
            codes[currChar] |= data[i]; //leading bit is 0, no need to mask
            currChar++;
            i++;

        //110xxxxx
        } else if ((data[i] & 192) == 192 && ((~data[i]) & 32) == 32) {
            // codes[currChar] = ((((uint32_t)data[i]) ^ 192) << 6) | (data[i + 1] ^ 128);
            codes[currChar] |= (((uint32_t)data[i]) ^ 192) << 6;
            codes[currChar] |= data[i + 1] ^ 128;
            currChar++;
            i += 2;

        //1110xxxx
        } else if ((data[i] & 224) == 224 && ((~data[i]) & 16) == 16) {
            // codes[currChar] = ((((uint32_t)data[i]) ^ 224) << 12) | ((((uint32_t)data[i+1]) ^ 128) << 6) | (((uint32_t)data[i+2]) ^ 128);
            codes[currChar] |= (((uint32_t)data[i]) ^ 224) << 12;
            codes[currChar] |= (((uint32_t)data[i+1]) ^ 128) << 6;
            codes[currChar] |= data[i+2] ^ 128;
            currChar++;
            i += 3;

        //11110xxx
        } else if ((data[i] & 240) == 240 && ((~data[i]) & 8) == 8) {
            // codes[currChar] = ((((uint32_t)data[i]) ^ 240) << 18) | ((((uint32_t)data[i+1]) ^ 128) << 12) | ((((uint32_t)data[i+2]) ^ 128) << 6) | (((uint32_t)data[i+3]) ^ 128);
            codes[currChar] |= (((uint32_t)data[i]) ^ 240) << 18;
            codes[currChar] |= (((uint32_t)data[i+1]) ^ 128) << 12;
            codes[currChar] |= (((uint32_t)data[i+2]) ^ 128) << 6;
            codes[currChar] |= data[i+3] ^ 128;
            currChar++;
            i += 4;
        
        //error
        } else {
            LOG_ERR("Something went horribly wrong parsing UTF8, leading bits don't make sense");
            displayErrorAndExit("Error encountered reading data file");
        }
    }
    /*
    * UTF8 CONVERSION END
    */
    
    //store
    result->ids = codes;
    result->length = numChars;
}

Text *getTextById(int id){
    if (id >= textTableLength || id < 0){
        LOG_ERR("Requested text id %d is outside of table size", id);
        displayErrorAndExit("Invalid data encountered");
    }
    
    return textTable + id;
}
