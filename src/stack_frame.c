#include "stack_frame.h"
#include "graphics.h"
#include "SDL2/SDL.h"
#include "input.h"
#include "logging.h"
#include "constants.h"
#include <math.h>

/////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////
typedef struct{
    float x;
    float y;
} Object;



/////////////////////////////////////////////////
// statics
/////////////////////////////////////////////////
static void drawObject(Object *obj);

/*
 * It might make more sense to center the room at 0 and rotate around that
 */
static const float drawOffset = 16*2;
static float center;

static Image *wallImage = NULL;
static Image *floorImage = NULL;
static Object *objectList = NULL;
static int numObjects = 26;
static const int numLayers = 10;
static float rotation = 0;

static float pitch = 1;

static Image *bufferImage = NULL;
/*
 * change this to change how the buffer is scaled - smaller means that we hide the stacking artifacts but everything is pixelated
 * should be between 1 and RENDER_SCALE_MULTIPLE, 2 seems to give a good balance for quality but still hiding artifacts
 * 1 would be the same as screen resolution, which frankly still sorta looks cool
 * 
 * I'm not totally sure, but it might be best to render to a large buffer, then down to a small buffer, then to the screen (or maybe
 * some scaling with the screen idk). I notice that some details are lost in the buffer (fine) BUT when you pitch the camera, since
 * its just stretching an image, the details don't change, and that doesn't quite look right?
 * (maybe try the buffer having some interpolation?)
 */
static const int bufferScale = 1;


void initStackFrame(){
    wallImage = loadImageFromFile("gfx/crate_top.png");
    floorImage = loadImageFromFile("gfx/floor.png");
    
    //16x16 objects forming a 7x7 perimeter
    center = drawOffset + 3*16 + 8;
    objectList = malloc(sizeof(Object) * numObjects);
    int i;
    for (i = 0; i < 7; i++){
        objectList[i].x = i * 16 + drawOffset;
        objectList[i].y = 0 + drawOffset;
    }
    for (i = 7; i < 13; i++){
        objectList[i].x = 0 + drawOffset;
        objectList[i].y = (i-7) * 16 + drawOffset;
    }
    for (i = 13; i < 19; i++){
        objectList[i].x = 16*6 + drawOffset;
        objectList[i].y = (i-13) * 16 + drawOffset;
    }
    for (i = 19; i < 26; i++){
        objectList[i].x = (i-19) * 16 + drawOffset;
        objectList[i].y = 16*6 + drawOffset;
    }
    
    // the buffer we draw to before stretching to the screen, normal width but triple height
    bufferImage = createEmptyImage(SCREEN_WIDTH * bufferScale, SCREEN_HEIGHT * bufferScale * 3);
}



/////////////////////////////////////////////////
// Logic
/////////////////////////////////////////////////
void doStackFrame(int delta){
    if (checkInput(LEFT_BUTTON)){
        rotation -= 2;
    } else if (checkInput(RIGHT_BUTTON)){
        rotation += 2;
    }
    
    //actually, this is not pitch but SCALING
    //the pitch should change linearly, as it is an angle, and the SCALING should change based on the trig for that, non-linearly
    //if constrained to a small interval it may not be noticeable
    if (checkInput(UP_BUTTON)){
        pitch += .1;
        printf("%f\n", pitch);
    } else if (checkInput(DOWN_BUTTON)){
        pitch -= .1;
        printf("%f\n", pitch);
    }
    
    //lock to some range
    //3 is the number of multiples of screen size that the buffer texture is, no coincidence, appears below with the draw scaling
    if (pitch <= .9){
        pitch = 0.9;
    } else if (pitch >= 3){
        pitch = 3;
    }
    
    float xScale;
    float yScale;
    SDL_Rect view;
    if (checkInput(A_BUTTON)){
        view.x = 0;
        view.y = 0;
        view.w = 64;
        view.h = 64;
        SDL_RenderSetViewport(renderer, &view);
        SDL_RenderSetScale(renderer, RENDER_SCALE_MULTIPLE*3, RENDER_SCALE_MULTIPLE*2);
    } else if (checkInput(B_BUTTON)){
        /*
         * Unfortunately, RenderSetScale scales things BEFORE they are rotated. We'd want it to scale afterwards.
         * We really only want to scale in the (true) y direction I think.
         * So to do this correctly, we need to render to a buffer texture, then scale that.
         */
        SDL_RenderSetViewport(renderer, NULL);
        SDL_RenderSetScale(renderer, RENDER_SCALE_MULTIPLE, RENDER_SCALE_MULTIPLE / pitch);
    } else {
        SDL_RenderSetViewport(renderer, NULL);
        SDL_RenderSetScale(renderer, RENDER_SCALE_MULTIPLE, RENDER_SCALE_MULTIPLE);
    }
    
}



/////////////////////////////////////////////////
// Drawing
/////////////////////////////////////////////////
void drawStackFrame(int excessTime){
    int i;
    int j;
    int k;
    Object obj;
    
    /*
     * Drawing one object at a time requires computing depth to the camera per-object 
     */
    //for (i = 0; i < numObjects; i++){
        //drawObject(objectList + i);
    //}
    
    int offsetX = 0;
    int offsetY = SCREEN_HEIGHT * (3-1);
    
    //change the target to be our buffer, clear the buffer
    SDL_SetRenderTarget(renderer, bufferImage->_texture);
    SDL_RenderClear(renderer);
    setDrawScaling(bufferScale); //scale up to match the buffer resolution
    
    //floor is not layered
    drawImageRotate(floorImage, 0 + drawOffset + offsetX, 0 + drawOffset + offsetY, rotation, center - (0 + drawOffset), center - (0 + drawOffset));
    
    
    /*
     * Drawing one LAYER at a time does not require computing depth to camera per-object
     * For billboarded sprites, just draw them repeatedly at each layer - sure you get overdraw, but it should work? As long as you draw back to front with the things in the layer I guess?
     * So I suppose it requires some 2d depth sorting, but it still avoids cycle problems with various depth algorithms
     */
    for (i = 0; i < numLayers; i++){
            
        //adds extra copies of the image to hide the gaps, although my theory is that if rendering at a low resolution, the gaps won't be visibile anyway
        //some rounding error here, probably because our draw methods don't take floats and you get some inconsistencies with draw positions
        for (j = 0; j < pitch; j++){
            for (k = 0; k < numObjects; k++){
                obj = objectList[k];
                drawImageRotate(wallImage, obj.x + offsetX, obj.y - (i * pitch) - j + offsetY, rotation, center - obj.x, center - obj.y);
            }
        }
    }
    
    //change target back to screen, draw stretched buffer
    SDL_SetRenderTarget(renderer, NULL);
    setDrawScaling(1); //no scaling
    
    SDL_Rect view;
    SDL_Rect dest;
    
    view.x = 0;
    view.y = SCREEN_HEIGHT * (3 - pitch) * bufferScale;
    view.w = SCREEN_WIDTH * bufferScale;
    view.h = SCREEN_HEIGHT * pitch * bufferScale;
    
    dest.x = 0;
    dest.y = (((1/pitch) - 1)* center) * RENDER_SCALE_MULTIPLE; //re-centers the image - WARNING: has problems when pitch is < 1, due to only drawing part of the buffer; probably need to clamp 1/pitch or something
    dest.w = WINDOW_WIDTH;
    dest.h = WINDOW_HEIGHT;
    
    SDL_RenderCopy(renderer, bufferImage->_texture, &view, &dest);
}

void drawObject(Object *obj){
    int i;
    int j;
    for (i = 1; i < numLayers; i++){
        //drawImageRotate(wallImage, obj->x, obj->y - i, rotation, center-obj->x, center-obj->y); //works w/o pitching
        
        //adds extra copies of the image to hide the gaps, although my theory is that if rendering at a low resolution, the gaps won't be visibile anyway
        for (j = 0; j < pitch; j++){
            drawImageRotate(wallImage, obj->x, obj->y - (i * pitch) - j, rotation, center-obj->x, center-obj->y);
        }
    }
}
