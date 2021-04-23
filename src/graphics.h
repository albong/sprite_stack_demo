#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "SDL2/SDL.h"



extern SDL_Renderer *renderer;



/*
 * Code both to manage SDL, and to manage our own graphics that are backed by SDL.
 * 
 * WARNING:
 * Currently we use cJSON to read in data files, and some of these files contain timings in
 * milliseconds.  cJSON only uses signed integers to read in files, meaning that it might be the
 * case that we are restricted to timings not exceeding ~32000 milliseconds, which is about
 * 32 seconds.  PROBABLY this isn't a problem, and we can work around this without having
 * to modify cJSON, but we haven't yet, so be aware of this.
 */


/////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////
//The base abstract type for all raster graphics in the engine
typedef struct Image{
    //What is actually used for drawing with SDL
    SDL_Texture *_texture;
    //not actually used, a holdover from when we used SDL1.  Sometimes surfaces are used for loading, but then they should be converted
    //at the moment this is always NULL, but I'm going to leave it as its easier to access pixel data through a surface than a texture, so if we need that its easy to modify
    SDL_Surface *_surface;
    //whether or not the texture is shared through a texture atlas
    int _isShared;
    //the underlying texture may actually contain more than just this image, so offset the draws to these coordinates
    int _x;
    int _y;
    //width of the image, which may not match the size of the underlying surface
    int width;
    int height;
} Image;

//It is possible when loading images to pack them all into a (hopefully) small number of images; to batch them for faster drawing
//Packed images have their _texture pointer changed to point to a shared image, and so we use this to hold pointers to those
//shared images so that we can easily free things.  Ideally this batching system would be opaque to the rest of the engine,
//but that requires tracking when image pointers are created and freed, which is doable, but more work.
typedef struct ImageAtlas{
    Image **images; //array of pointers
    int numImages;
} ImageAtlas;

//Used for specifying subsections of an image.  Uses pixels
typedef struct ImageRect{
    int x;
    int y;
    int w;
    int h;
} ImageRect;

//Probably ought to be called a spritesheet properly - an image along with data about how to cut up said image into rectangles,
//each rectangle being a frame.  Frames are numbered starting from 0, right and down
typedef struct Sprite{
    Image *image;
    int frameWidth;
    int frameHeight;
    int numFramesPerRow;
} Sprite;

//This is how we track animations.  Animations are composed of various loops (not necessarily looping) of frames
//and timings for how long to display frames for.  Loop and frame indices are used to find what index in a sprite to
//draw from (or hitbox to use).
typedef struct Animation{
    //index of current loop being played
    int currLoop;
    //total number of loops
    int numLoops;
    //number of milliseconds passed in the current loop (reset at loop end), functions effectively as the "current time" in the loop
    int milliPassed;
    //the index of the current frame of the current loop
    int currFrame;
    //number of frames per loop
    int *loopLength;
    //array of arrays of times in milliseconds when to start playing each frame
    int **frameStartTime;
    //time in milliseconds when the current loop should end, also functions as the duration of the loop
    int *loopEndTime;
    //flag indicating whether or not to repeat a loop
    int *repeatLoop;
    //2d array of indices (right and down) of sections of the sprite sheet; first index is the loop index, second index is the frame index
    int **spriteIndices;
} Animation;


/////////////////////////////////////////////////
// SDL
/////////////////////////////////////////////////
void initSDL();
void stopSDL();
int showErrorMessageBoxSDL(const char *message); //don't call directly, use omni_exit to display an error and exit


/////////////////////////////////////////////////
// Init
/////////////////////////////////////////////////
Image *init_Image(Image *self);
Sprite *init_Sprite(Sprite *self);
Animation *init_Animation(Animation *self);


/////////////////////////////////////////////////
// Free
/////////////////////////////////////////////////
void free_Image(Image *self);
void free_ImageAtlas(ImageAtlas *self);
void free_Sprite(Sprite *self);
void free_Animation(Animation *self);


/////////////////////////////////////////////////
// Loading
/////////////////////////////////////////////////
/*
 * Creating or loading images will crash the game if the filename is wrong or if SDL cannot create the image
 */
Image *createEmptyImage(int width, int height);
Image *loadImageFromFile(char *filename);
void deepCopy_Animation(Animation *to, Animation *from);
Animation *shallowCopyAnimation(Animation *original);
void startBatchingLoadedImages(); //1 on success, 0 on failure
ImageAtlas *stopBatchingLoadedImages(); //1 on success, 0 on failure
Animation *getNoAnimation(); //creates an animation for something that isn't animated but needs the animation for hitboxes or whatever, one loop with one frame


/////////////////////////////////////////////////
// Animation management
/////////////////////////////////////////////////
int updateAnimation(Animation *self, int delta);
void setAnimationLoop(Animation *self, int loop, int forceRestart);


/////////////////////////////////////////////////
// Drawing
/////////////////////////////////////////////////
/*
 * All of these will crash the game if the SDL calls they depend on fail
 */
void drawImage(Image *image, int x, int y);
void drawImageRotate(Image *image, float x, float y, float angle, int centerX, int centerY);
void drawImageToImage(Image *src, Image *dst, ImageRect *srcRect, ImageRect *dstRect); //either rectangle is allowed to be null for full size
void drawImageSrcDst(Image *image, ImageRect *srcRect, ImageRect *dstRect); //either rectangle is allowed to be null for full size
void drawUnfilledRect(int x, int y, int w, int h, int r, int g, int b);
void drawFilledRect(int x, int y, int w, int h, int r, int g, int b);
void drawFilledRectA(int x, int y, int w, int h, int r, int g, int b, int a);
void drawLine(int x1, int y1, int x2, int y2, int r, int g, int b);
void drawAnimation(Sprite *s, Animation *anim, int x, int y); //anim can be null to just draw entire sprite


/////////////////////////////////////////////////
// Screen management
/////////////////////////////////////////////////
/*
 * clearScreen will crash the game if the SDL call it depends on fails
 */
void clearScreen();
void bufferToScreen();
void setDrawScaling(int scaling);

#endif
