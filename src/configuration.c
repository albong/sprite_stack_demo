#include "configuration.h"
#include "logging.h"
#include "omni_exit.h"
#include "file_reader.h"
#include "../lib/cjson_wrapper.h"
#include "SDL2/SDL_log.h"

/////////////////////////////////////////////////
// "Constants"
/////////////////////////////////////////////////
int DEBUG_DRAW_BOUNDING_BOXES = 0;
int DEBUG_DRAW_ONLY_BOUNDING_BOXES = 0;
int DEBUG_DRAW_ATTACK_HITBOX = 0;
int DEBUG_DRAW_HURT_HITBOX = 0;
int DEBUG_DRAW_INTERACT_HITBOX = 0;
int DEBUG_DRAW_PHYSICAL_HITBOX = 0;
int DEBUG_DRAW_WALL_HITBOX = 0;
int DEBUG_DRAW_FRAMERATE = 0;

//booleans, 0 or 1
int DEBUG_SKIP_INITIAL_TITLE_SCREEN = 0;
int DEBUG_USE_DEBUG_NEWGAME_FILE = 0;


/////////////////////////////////////////////////
// Loading
/////////////////////////////////////////////////
void loadConfiguration(){
    cJSON *root = NULL;
    cJSON *debugDraw = NULL;
    char *fileContents = NULL;
    int errorCount = 0;
    const char *filename = "data/configuration.data";
    
    fileContents = readFileToString(filename);
    root = cJSON_Parse(fileContents);
    if (!root){
        LOG_ERR("%s: Error before: %s\n", filename, cJSON_GetErrorPtr());
        displayErrorAndExit("Encountered a problem when reading datafile %s", filename);
    }
    
    //Level 0: log debug, info, warnings, and errors
    //Level 1: log info, warnings, and errors
    //Level 2: log warnings and errors
    //Level 3: log errors
    int loggingLevel = cjson_readInt(root, "logging level", &errorCount);
    if (loggingLevel <= 0) {
        SDL_LogSetPriority(SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_DEBUG);
    } else if (loggingLevel == 1){
        SDL_LogSetPriority(SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_INFO);
    } else if (loggingLevel == 2){
        SDL_LogSetPriority(SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_WARN);
    } else if (loggingLevel >= 3){
        SDL_LogSetPriority(SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_ERROR);
    }
    
    DEBUG_SKIP_INITIAL_TITLE_SCREEN = cjson_readBoolean(root, "debug skip initial title screen", &errorCount);
    DEBUG_USE_DEBUG_NEWGAME_FILE = cjson_readBoolean(root, "debug use debug newgame file", &errorCount);;
    
    if (cjson_readBoolean(root, "enable debug drawing", &errorCount)){
        debugDraw = cjson_readObject(root, "debug drawing settings", &errorCount);
        
        DEBUG_DRAW_BOUNDING_BOXES = cjson_readBoolean(debugDraw, "draw bounding boxes", &errorCount);
        DEBUG_DRAW_ONLY_BOUNDING_BOXES = cjson_readBoolean(debugDraw, "draw only bounding boxes", &errorCount);
        DEBUG_DRAW_ATTACK_HITBOX = cjson_readBoolean(debugDraw, "draw attack hitboxes", &errorCount);
        DEBUG_DRAW_HURT_HITBOX = cjson_readBoolean(debugDraw, "draw hurt hitboxes", &errorCount);
        DEBUG_DRAW_INTERACT_HITBOX = cjson_readBoolean(debugDraw, "draw interact hitboxes", &errorCount);
        DEBUG_DRAW_PHYSICAL_HITBOX = cjson_readBoolean(debugDraw, "draw physical hitboxes", &errorCount);
        DEBUG_DRAW_WALL_HITBOX = cjson_readBoolean(debugDraw, "draw wall hitboxes", &errorCount);
        
        DEBUG_DRAW_FRAMERATE = cjson_readBoolean(debugDraw, "draw framerate", &errorCount);
    }
    
    if (errorCount > 0){
        LOG_ERR("Encountered a problem reading configuration file %s", filename);
        displayErrorAndExit("Encountered a problem reading configuration file");
    }
}
