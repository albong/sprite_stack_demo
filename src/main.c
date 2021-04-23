#include "graphics.h"
#include "input.h"
#include "frames.h"
#include "configuration.h"
#include "constants.h"
#include "logging.h"
#include "omni_exit.h"
#include "stack_frame.h"
#include "font.h"

#include <stdio.h>
#include <stdlib.h>

//Refer to http://gamedev.stackexchange.com/a/132835 for changes

static void initializeOmnisquash();
static void terminateOmnisquash();

int main(int argc, char *argv[]){
    initializeOmnisquash();
    gameLoop();
    terminateOmnisquash();
    return 0;
}

void initializeOmnisquash(){
    //don't buffer standard out, makes logging easier
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
    loadConfiguration();
    
    initSDL();
//	freopen( "CON", "w", stdout );
//    freopen( "CON", "w", stderr );
    
    //when the program exits, clean everything up
    atexit(stopSDL);
    installSegfaultHandler();

    //initialization stuff
    //initSound();
    //initWeaponLists(); //creates the player's arrays, must be done before the area and player
    //initInventory(); //should be done before player
    //initPlayer();
    //initMenu();
    initInput();
    //initTextbox();
    //initInterface(); //no interface in apt 345
    //initTitleScreen();
    //initLoadScreen();
    //initGlobalFlagTable();
    //initCollisions();

    //set the current font stuff
    //setCurrentLanguage(EN_LC);
    initFonts();
    //loadTextForCurrentLanguage();
    initStackFrame();
 
    initFrames();
}

void terminateOmnisquash(){
    clearScreen();
    
    //free everything left
    //termSound();
    //termWeaponLists();
    //termInventory();
    //termPlayer();
    //termMenu();
    //nothing to terminate with input
    //termTextbox();
    //termInterface(); //no interface in apt 345
    //termTitleScreen();
    //termLoadScreen();
    //termGlobalFlagTable();
    //termCollisions();
    
    //termText();
    termFonts();
    
    termFrames();
}
