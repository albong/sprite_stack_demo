#ifndef LOGGING_H
#define LOGGING_H

#include "constants.h"
#include "SDL2/SDL_log.h"
#include <stdio.h>

/*
 * Used to print things out from inside the engine. Used to be straight printing,
 * now uses SDL's logging. Whether or not a log actually prints is set by configuration.
 * 
 * For some reason I wrote these as macros instead of functions.
 */


//DEB=DEBUG, INF=INFO, WAR=WARNING, ERR=ERROR.  I have a thing about equal lengths.
//the '##' before __VA_ARGS__ does some token nonsense that makes things work if there are no arguments
//removes the prior comma essentially.  M A G I C
//#if LOGGING_LEVEL == 0
    //#define LOG_DEB(fmt, ...) printf("DEBUG %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
//#else
    //#define LOG_DEB(fmt, ...)
//#endif

//#if LOGGING_LEVEL <= 1
    //#define LOG_INF(fmt, ...) printf("INFO %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
//#else
    //#define LOG_INF(fmt, ...)
//#endif

//#if LOGGING_LEVEL <= 2
    //#define LOG_WAR(fmt, ...) printf("WARNING %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
//#else
    //#define LOG_WAR(fmt, ...)
//#endif

//#define LOG_ERR(fmt, ...) printf("ERROR %s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_DEB(fmt, ...) SDL_LogDebug(SDL_LOG_CATEGORY_CUSTOM, "%s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_INF(fmt, ...) SDL_LogInfo(SDL_LOG_CATEGORY_CUSTOM, "%s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WAR(fmt, ...) SDL_LogWarn(SDL_LOG_CATEGORY_CUSTOM, "%s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "%s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#endif
