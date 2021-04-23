#include "file_reader.h"
#include "logging.h"
#include "omni_exit.h"

#include "SDL2/SDL_platform.h"
#include "SDL2/SDL_rwops.h"

#include <stdlib.h>

#ifdef __WINDOWS__
    #include <io.h>
#else
    #include <unistd.h>
#endif


char *readFileToString(const char *filename){
    /*
     * More or less copied and pasted from the cJSON examples
     * Can easily swap the SDL stuff for FILE and fread and etc.
     */
    SDL_RWops *f;
    unsigned long len;
    char *data;

    // open in read binary mode
    f = SDL_RWFromFile(filename, "rb");
    if (f == NULL){
        LOG_ERR("Unable to read file %s to string: %s", filename, SDL_GetError());
        displayErrorAndExit("Problem reading data file");
    }
    
    // get the length
    SDL_RWseek(f, 0, RW_SEEK_END);
    len = SDL_RWtell(f);
    SDL_RWseek(f, 0, RW_SEEK_SET);

    //allocate and read the data
    data = malloc(len + 1);
    SDL_RWread(f, data, 1, len);
    SDL_RWclose(f);
    
    //add a null character cause its a string
    data[len] = '\0';
    
    return data;
}

unsigned char *readBinaryFileToCharStar(const char *filename, unsigned long *length){
    /*
     * Properly I guess unsigned long long or size_t is more appropriate than unsigned long
     * 
     * More or less copied and pasted from the cJSON examples
     * Can easily swap the SDL stuff for FILE and fread and etc.
     */
    SDL_RWops *f;
    unsigned long len;
    char *data;

    // open in read binary mode
    f = SDL_RWFromFile(filename, "rb");
    if (f == NULL){
        LOG_ERR("Unable to read file %s to string: %s", filename, SDL_GetError());
        displayErrorAndExit("Problem reading data file");
    }
    
    // get the length
    SDL_RWseek(f, 0, RW_SEEK_END);
    len = SDL_RWtell(f);
    SDL_RWseek(f, 0, RW_SEEK_SET);

    //allocate and read the data
    data = malloc(len);
    SDL_RWread(f, data, 1, len);
    SDL_RWclose(f);
    
    //store the length
    if(length != NULL){
        *length = len;
    }
    
    return data;
}

int fileExists(const char *filename){
    int result = 0;
    
    if (filename != NULL){
        #ifdef __WINDOWS__
            result = !_access(filename, 0); //mode 0 checks for existence
        #else
            result = !access(filename, 0); //mode 0 checks for existence
        #endif
    }
    
    return result;
}
