#include "omni_exit.h"
#include "graphics.h"
#include "logging.h"
#include <stdio.h> 
#include <stdarg.h>
#include <stdlib.h> // for exit()
#include <signal.h>
#include <SDL2/SDL.h>

//needed to store the original signal handler
//there's a type in the gnu libc but not on windows so...
//thanks to: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/signal?view=msvc-160
typedef void (*SignalHandlerPointer)(int);
static SignalHandlerPointer defaultHandler;
int defaultHandlerStored = 0;

static void segfaultHandler(int sig);

void displayErrorAndExit(const char *format, ...){
    int messageLength;
    char *message;
    va_list args;
    
    va_start(args, format);
    messageLength = vsnprintf(NULL, 0, format, args) + 1; //+1 for null character
    va_end(args);
    
    message = malloc(sizeof(char) * messageLength);

    va_start(args, format);
    vsprintf(message, format, args);
    va_end(args);
    
    showErrorMessageBoxSDL(message); //returns a code for success or fail but we're exiting so who cares
    
    LOG_ERR("%s", message);
    LOG_ERR("Exiting");
    exit(1);
}

void installSegfaultHandler(){
    if (!defaultHandlerStored){
        defaultHandler = signal(SIGSEGV, segfaultHandler);
        defaultHandlerStored = 1;
    }
}

void segfaultHandler(int sig){
    //thanks to: https://stackoverflow.com/a/13290134
    //we have to reset the default handler after doing our stuff and re-raise the error to trigger the segfault
    //since apparently the default is not actually a function or something, SIG_DFL is probably a number
    
    //reinstall the original handler, then try to print a message to the user
    //if the segfault breaks SDL, this way THAT failure will cause an abort, rather than
    //possibly getting into an infinite segfault loop
    signal(sig, SIG_DFL);
    showErrorMessageBoxSDL("Segmenation fault occurred");
    raise(sig);
}
