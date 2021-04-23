#ifndef TEXT_H
#define TEXT_H
#include <stdint.h>
#include <stddef.h>

/*
 * Responsible for text in the engine.  Not textboxes or fonts, but the actual text.
 * 
 * All text is read from file at load time, since it was too difficult to figure out
 * how to cleverly only load the text that was needed.  There's a script that processes the text file
 * to create a supplementary json file that eases loading by doing the preprocessing to
 * determine where blocks of text start and end (called the manifest file).
 * 
 * Text is encoded with UTF8, for language support.  On changing the language, all
 * text has to be reloaded.
 */


/////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////
typedef struct Text {
    //ids are unicode points in decimal
    uint32_t *ids;
    //number of ids
    size_t length;
} Text;


/////////////////////////////////////////////////
// Enums
/////////////////////////////////////////////////
//used to track what language's text is currently loaded
typedef enum {
    EN_LC, KO_LC, JP_LC
} LanguageCode;


/////////////////////////////////////////////////
// Loading/Unloading
/////////////////////////////////////////////////
void termText();
Text *init_Text(Text *self);
void free_Text(Text *self);
void loadTextForCurrentLanguage();


/////////////////////////////////////////////////
// Access
/////////////////////////////////////////////////
void setCurrentLanguage(LanguageCode code);
LanguageCode getCurrentLanguage();
const char *getCurrentLanguageString();
Text *getTextById(int id);

#endif
