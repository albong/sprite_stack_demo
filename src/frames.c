#include "frames.h"
#include "input.h"
#include "constants.h"
#include "debug_drawer.h"
#include "logging.h"
#include "omni_exit.h"
#include "graphics.h"
#include "stack_frame.h"

#include "SDL2/SDL.h"

#include <stdlib.h>
#include <stdio.h>


// Enums
typedef enum {
    NONE_FN,
    GAME_FN,
    MENU_FN,
    TEXTBOX_FN,
    TITLE_SCREEN_FN,
    LOAD_SCREEN_FN,
    SCREEN_TRANSITION_FN,
    CUTSCENE_FN,
    STACK_FN
} FrameName;


// Structs
typedef struct Frame{
    void (* logic)(int delta);
    void (* draw)(int excessTime);
    //a flag for whether or not to draw the frame even if it isn't the top of the stack
    int drawIfNotTop;
    //a flag to indicate if the game loop should NOT draw after popping this frame until another game step has passed
    //useful if the frame being popped to needs to perform logic/push something else before the next draw in order for things to make sense
    int delayDrawAfterPop;
    char *name;
} Frame;


//Variables
static Frame *gameFrame;
static Frame *menuFrame;
static Frame *textboxFrame;
static Frame *titleScreenFrame;
static Frame *loadScreenFrame;
static Frame *transitionFrame;
static Frame *cutsceneFrame;
static Frame *stackFrame;
//these are functionally private, in the sense that you shouldn't access them except in the game loop or their accessor methods
//kinda overkill but seems like a good way to keep the frame logics readable and sensible
static Frame *_framesToPushOnStack[FRAME_STACK_SIZE];
static int _numberOfFramesToPushOnStack = 0;
static int _numberOfFramesToPopOffStack = 0;
static Frame *_frameToPopTo = NULL;


//methods
static Frame *allocateFrame(void (* logic)(int delta), void (* draw)(int excessTime), int drawIfNotTop, int delayDrawAfterPop, char *name);
static void stackFrameLogic(int delta);
static void stackFrameDraw(int excessTime);

static void pushFrameOntoStack(Frame *toPush);
static void popFramesFromStack(int numberOfFramesToPop);
static void popToFrame(Frame *toPopTo);


/////////////////////////////////////////////////
// Loading/Unloading
/////////////////////////////////////////////////
void initFrames(){
    stackFrame       = allocateFrame(&stackFrameLogic,       &stackFrameDraw,       1, 0, "stacking");
    
    int i;
    for (i = 0; i < FRAME_STACK_SIZE; i++){
        _framesToPushOnStack[i] = NULL;
    }
    _numberOfFramesToPushOnStack = 0;
    _numberOfFramesToPopOffStack = 0;
    _frameToPopTo = NULL;
}

Frame *allocateFrame(void (* logic)(int delta), void (* draw)(int delta), int drawIfNotTop, int delayDrawAfterPop, char *name){
    Frame *result = malloc(sizeof(Frame));
    result->logic = logic;
    result->draw = draw;
    result->drawIfNotTop = drawIfNotTop;
    result->delayDrawAfterPop = delayDrawAfterPop;
    result->name = name;
    return result;
}

void termFrames(){
    //don't free the names, they're not malloc'd
    
    free(gameFrame);
    free(menuFrame);
    free(textboxFrame);
    free(titleScreenFrame);
    free(loadScreenFrame);
    free(transitionFrame);
}


/////////////////////////////////////////////////
// Frame Logic
/////////////////////////////////////////////////
void stackFrameLogic(int delta){
    if (checkInput(ESCAPE_BUTTON)){
        consumeAllInput();
        popFramesFromStack(1);
        return;
    }
    
    doStackFrame(delta);
}

void stackFrameDraw(int excessTime){
    //setDrawScaling(RENDER_SCALE_MULTIPLE); //scale up to window resolution
    drawStackFrame(excessTime);
}


/////////////////////////////////////////////////
// Game Logic
/////////////////////////////////////////////////
void gameLoop(){
    //SDL_GetTicks returns Uint32, but we want to use int, so we only use Uint32 for the result of SDL_GetTicks calls
    Uint32 currMilliseconds = 0;
    Uint32 prevMilliseconds = 0;
    int delta = 0;
    int accumulator = 0;
    int excessTime = 0;
    int numberOfLogicFramesRun = 0;
    int frameStackChanged = 0; //don't interpolate drawing if the frame stack changed last tick
    int delayDraw = 0; //wait until another logic step before drawing again
    
    int runGameLoop = 1;
    int i;
    
    int currFrame = 0;
    int numFrames = 1;
    Frame *frames[FRAME_STACK_SIZE]; //this is hardcoded, just increase to match number of possible frames, as this will be small
    frames[0] = stackFrame;
    
    //see:
    //https://gameprogrammingpatterns.com/game-loop.html
    //https://gafferongames.com/post/fix_your_timestep/
    currMilliseconds = SDL_GetTicks() - FRAME_TIMESTEP_MILLISECONDS; //has to be here otherwise we track time from game initialization, which is ~500 milliseconds on this computer, way too long
    while (runGameLoop){
        //calculate the time delta - EVENTUALLY (after a month) the time will wrap, so I GUESS we should handle that
        prevMilliseconds = currMilliseconds;
        currMilliseconds = SDL_GetTicks();
        if (currMilliseconds >= prevMilliseconds){
            delta = (int)(currMilliseconds - prevMilliseconds);
        } else {
            delta = (int)((SDL_MAX_UINT32 - currMilliseconds) + prevMilliseconds);
        }
        
        //logic
        accumulator += delta;//must be incremented after the frame capping so as to not double count deltas slightly
        numberOfLogicFramesRun = 0;
        while (accumulator >= FRAME_TIMESTEP_MILLISECONDS && numberOfLogicFramesRun <= MAX_FRAMES_BEFORE_FORCE_DRAW){
            frameStackChanged = 0;
            delayDraw = 0;
            numberOfLogicFramesRun++;
            accumulator -= FRAME_TIMESTEP_MILLISECONDS;
            
            //frame logic
            getInput();
            frames[currFrame]->logic(FRAME_TIMESTEP_MILLISECONDS);
            
            //update frame stack
            if (_numberOfFramesToPushOnStack > 0){
                for (i = 0; i < _numberOfFramesToPushOnStack; i++){
                    numFrames++;
                    currFrame++;
                    frames[currFrame] = _framesToPushOnStack[i];
                    _framesToPushOnStack[i] = NULL;
                }
                _numberOfFramesToPushOnStack = 0;
                frameStackChanged = 1;
            } else if (_numberOfFramesToPopOffStack > 0){
                for (i = 0; i < _numberOfFramesToPopOffStack; i++){
                    numFrames--;
                    currFrame--;
                }
                _numberOfFramesToPopOffStack = 0;
                frameStackChanged = 1;
                
                //check if the last frame popped wants to delay drawing
                //works because the array hasn't changed, just because our indexing meta-data
                if (frames[currFrame + 1]->delayDrawAfterPop){
                    delayDraw = 1;
                }
                
                //if we've popped all frames, break out of the loop
                if (currFrame < 0){
                    runGameLoop = 0;
                    break;
                }
            } else if (_frameToPopTo != NULL){
                for (i = currFrame; i >= 0; i--){
                    if (frames[i] != _frameToPopTo){
                        numFrames--;
                        currFrame--;
                    } else {
                        break;
                    }
                }
                if (numFrames == 0){
                    LOG_ERR("Tried to pop to frame '%s' but it wasn't on the stack!", _frameToPopTo->name);
                    displayErrorAndExit("An invalid display state has been reached");
                }
                
                //check if the last frame popped wants to delay drawing
                //works because the array hasn't changed, just because our indexing meta-data
                if (frames[currFrame + 1]->delayDrawAfterPop){
                    delayDraw = 1;
                }
                
                _frameToPopTo = NULL;
                frameStackChanged = 1;
            }
        }
        
        //skip the drawing if we're delaying the draw
        if (delayDraw){
            continue;
        }
        
        /*
         * Determine if we should interpolate:
         *  1) Don't interpolate if we're in a death sprial because it will not be accurate
         *  2) Don't interpolate if we just changed the frame stack because we will get jitters due to being out of sync
         */
        if ((numberOfLogicFramesRun > MAX_FRAMES_BEFORE_FORCE_DRAW) || frameStackChanged){
            excessTime = 0;
        } else {
            excessTime = accumulator;
        }
        
        //draw
        clearScreen();
        for (i = 0; i < numFrames; i++){
            if (i == numFrames-1){
                frames[i]->draw(excessTime);
            } else if (frames[i]->drawIfNotTop){
                //we don't want to interpolate for anything but the top frame
                //we can only change frames at the end of a logic step so no interpolation is probably fine and any slight wrong positioning shouldn't be noticeable
                frames[i]->draw(0);
            } else {
                continue;
            }
        }
        setDrawScaling(1); //UI scale for the framerate
        debugComputeAndDisplayFramerate(delta);//here we actually use delta because we want refresh rate
        bufferToScreen();
    }
}

void pushFrameOntoStack(Frame *toPush){
    _numberOfFramesToPushOnStack++;
    _framesToPushOnStack[_numberOfFramesToPushOnStack - 1] = toPush;
}

void popFramesFromStack(int numberOfFramesToPop){
    _numberOfFramesToPopOffStack += numberOfFramesToPop;
}

void popToFrame(Frame *toPopTo){
    //this works because each instance of the frame is unique
    _frameToPopTo = toPopTo;
}
