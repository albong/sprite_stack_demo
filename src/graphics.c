#include "graphics.h"
#include "constants.h"
#include "logging.h"
#include "omni_exit.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static ImageAtlas *init_ImageAtlas(ImageAtlas *self);
static SDL_Surface *loadSurfaceFromFile(char *filename); //crashes on read failure 
static SDL_Texture *loadTextureFromFile(char *filename); //crashes on read failure 
static void addImageToBatchIfBatching(Image *toBeBatched);


/////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////
static SDL_Window *window = NULL;
//static SDL_Renderer *renderer = NULL;
SDL_Renderer *renderer = NULL;

//the max sizes will fit in int because thats what sdl returns
static int maxTextureWidth;
static int maxTextureHeight;

static int isBatching = 0;
static Image **imagesToBatch = NULL; //array of pointers
static int numberOfImagesToBatch = 0;


/////////////////////////////////////////////////
// SDL
/////////////////////////////////////////////////
void initSDL(){
    //Initialise SDL - the audio extension library is initialized elsewhere, but we must flag here
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0){
		LOG_ERR("Could not initialize SDL: %s", SDL_GetError());
		displayErrorAndExit("Problem setting up graphics");
	}
	
    //setup the window name
    char nameBuffer[100];
    if (IS_DEVELOPMENT_VERSION){
        sprintf(nameBuffer, "%s %d.%d.%d - omnisquash %d.%d.%d devel", 
            GAME_NAME, 
            GAME_VERSION_MAJOR, GAME_VERSION_MINOR, GAME_VERSION_PATCH,
            ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH
        );
    } else {
        sprintf(nameBuffer, "%s %d.%d.%d - omnisquash %d.%d.%d", 
            GAME_NAME, 
            GAME_VERSION_MAJOR, GAME_VERSION_MINOR, GAME_VERSION_PATCH,
            ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH
        );
    }
    
	//Open a screen
    window = SDL_CreateWindow(nameBuffer, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	if (window == NULL){
		LOG_ERR("Couldn't set screen mode to %d x %d: %s", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_GetError());
		displayErrorAndExit("Problem setting up graphics");
	}

    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1"); //WARNING: technically does nothing since we are not picky about the renderer and is enabled by default then, but could cause problems if we directly access the platform
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC); //I added flags, may break stuff, idk
    if (renderer == NULL){
        LOG_ERR("Couldn't create a renderer: %s", SDL_GetError());
        displayErrorAndExit("Problem setting up graphics");
    }
    SDL_RenderSetIntegerScale(renderer, 1);
    //SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    //store the max size for textures, clamped to something reasonable
    //it turned out that if very large sizes were used, we got stuttering for some reason, so the values for the constants may require tuning
    SDL_RendererInfo rendererInfo;
    if (SDL_GetRendererInfo(renderer, &rendererInfo)){
        LOG_ERR("Failed to query the renderer: %s", SDL_GetError());
        displayErrorAndExit("Problem setting up graphics");
    }
    maxTextureWidth = rendererInfo.max_texture_width;
    maxTextureHeight = rendererInfo.max_texture_height;
    maxTextureWidth = (maxTextureWidth > MAX_TEXTURE_WIDTH) ? MAX_TEXTURE_WIDTH : maxTextureWidth;
    maxTextureHeight = (maxTextureHeight > MAX_TEXTURE_HEIGHT) ? MAX_TEXTURE_HEIGHT : maxTextureHeight;

    //Initialize SDL_image
    int imageInitResult = IMG_Init( IMG_INIT_PNG );
    if (!(imageInitResult & IMG_INIT_PNG)){
        LOG_ERR("Couldn't initialize SDL_image!");
        displayErrorAndExit("Problem setting up graphics");
    }
}

void stopSDL(){
    SDL_Quit();
}

int showErrorMessageBoxSDL(const char *message){
    //
    //this is only here and not in omni_exit because we need the parent window to attach to and I don't want to expose that
    //
    return SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", message, window);
}


/////////////////////////////////////////////////
// Init
/////////////////////////////////////////////////
Image *init_Image(Image *self){
    self->_texture = NULL;
    self->_surface = NULL;
    self->_isShared = 0;
    self->_x = 0;
    self->_y = 0;
    self->width = 0;
    self->height = 0;
    
    return self;
}

ImageAtlas *init_ImageAtlas(ImageAtlas *self){
    self->images = NULL;
    self->numImages = 0;
    
    return self;
}

Sprite *init_Sprite(Sprite *self){
    self->image = NULL;
    self->frameWidth = 0;
    self->frameHeight = 0;
    self->numFramesPerRow = 0;

    return self;
}

Animation *init_Animation(Animation *self){
    self->currLoop = 0;
    self->numLoops = 0;
    self->milliPassed = 0;
    self->currFrame = 0;
    self->loopLength = NULL;
    self->frameStartTime = NULL;
    self->loopEndTime = NULL;
    self->repeatLoop = NULL;
    self->spriteIndices = NULL;
    
    return self;
}


/////////////////////////////////////////////////
// Free
/////////////////////////////////////////////////
void free_Image(Image *self){
    if (self == NULL){
        LOG_WAR("Tried to free NULL image");
        return;
    }
    
    //free SDL stuff only if it is not shared with another image
    if (!self->_isShared){
        SDL_DestroyTexture(self->_texture); //not safe to pass NULL
        SDL_FreeSurface(self->_surface); //safe to pass NULL
    }
    
    self->_surface = NULL;
    self->_texture = NULL;
    self->_isShared = 0;
    self->_x = 0;
    self->_y = 0;
    self->width = 0;
    self->height = 0;
    
    free(self);
}

void free_ImageAtlas(ImageAtlas *self){
    if (self == NULL){
        LOG_WAR("Tried to free NULL ImageAtlas");
        return;
    }
    
    int i;
    for (i = 0; i < self->numImages; i++){
        free_Image(self->images[i]);
    }
    free(self->images);
    self->images = NULL;
    self->numImages = 0;
    
    free(self);
}

void free_Sprite(Sprite *self){
    if (self == NULL){
        LOG_WAR("Received null pointer");
        return;
    }
    
    free_Image(self->image);
    self->image = NULL;
    self->frameWidth = 0;
    self->frameHeight = 0;
    self->numFramesPerRow = 0;
    free(self);
}

void free_Animation(Animation *self){
    if (self == NULL){
        LOG_WAR("Received null pointer");
        return;
    }

    int i;
    for (i = 0; i < self->numLoops; i++){
        free(self->spriteIndices[i]);
        free(self->frameStartTime[i]);
        self->spriteIndices[i] = NULL;
        self->frameStartTime[i] = NULL;
    }
    free(self->loopLength);
    free(self->loopEndTime);
    free(self->repeatLoop);
    free(self->spriteIndices);
    free(self->frameStartTime);
    self->loopLength = NULL;
    self->loopEndTime = NULL;
    self->repeatLoop = NULL;
    self->spriteIndices = NULL;
    self->frameStartTime = NULL;
    
    self->currLoop = 0;
    self->numLoops = 0;
    self->milliPassed = 0;
    self->currFrame = 0;
    
    free(self);
}


/////////////////////////////////////////////////
// Loading
/////////////////////////////////////////////////
SDL_Surface *loadSurfaceFromFile(char *filename){
    SDL_Surface* loadedImage = NULL;
    
    //Load the image
    loadedImage = IMG_Load(filename);
    if( loadedImage == NULL ){
        LOG_ERR("Failed to load image at %s", filename);
        displayErrorAndExit("Failed to load graphics file");
    }
    
    //Map the color key
    Uint32 colorkey = SDL_MapRGB(loadedImage->format, 0xFF, 0, 0xFF); //magic pink
    
    //Set all pixels of color R 0, G 0xFF, B 0xFF to be transparent
    SDL_SetColorKey(loadedImage, SDL_TRUE, colorkey);

    return loadedImage;
}

SDL_Texture *loadTextureFromFile(char *filename){
    /*
     * Textures are hardware accelerated and surfaces are not, so we use textures, but
     * to get a texture you must load a surface and convert it. We don't need the surface
     * afterwards though.
     */

    SDL_Surface *surface = loadSurfaceFromFile(filename);
    SDL_Texture *result = SDL_CreateTextureFromSurface(renderer, surface);
    if (result == NULL){
        LOG_ERR("Problem converting an SDL_Surface to an SDL_Texture!");
        displayErrorAndExit("Graphics error encountered");
    }
    
    SDL_FreeSurface(surface);
    return result;
}

Image *createEmptyImage(int width, int height){
    if (width < 0 || width > maxTextureWidth || height < 0 || height > maxTextureHeight){
        LOG_ERR("Invalid size requested for empty image");
        displayErrorAndExit("Graphics error encountered");
    }
    
    Image *result = init_Image(malloc(sizeof(Image)));
    result->width = width;
    result->height = height;
    
    //create a blank texture that can be edited
    result->_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);;
    
    //if the texture was not created, return null
    if (result->_texture == NULL){
        LOG_ERR("Failed to create empty texture");
        displayErrorAndExit("Graphics error encountered");
    }
        
    //make transparent - oh my goodness so convoluted
    //thanks to riptor's december 1st response (near the end): https://forums.libsdl.org/viewtopic.php?p=40949
    
    //change the renderer to this texture and ready to draw transparency
    SDL_SetTextureBlendMode(result->_texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, result->_texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
    
    SDL_RenderClear(renderer);
    
    //restore old draw mode and return to rendering the window
    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    
    //after reading documentation, here's something important to know:
    //the texture is rendered based on its blend mode; with SDL_BLENDMODE_BLEND, this will mix the source and dest alpha.
    //we don't really want that?  but if we just stick to non-transparent textures, it will be fine.  I'm not sure why
    //setting the blend mode to none (use the source alpha) doesn't work, except that it doesn't.
    //so this might cause bugs, but oh well.
    //it does seem like if we maybe used SDL_RenderCopy to move our transparent texture to a new one, that might work.
    //but whatever.
    //iirc, creating a transparent surface and converting it gives a texture with the wrong mode - appropriate for loading, not for creating buffers.
    
    addImageToBatchIfBatching(result);
    return result;
}

Image *loadImageFromFile(char *filename){
    /*
     * Even though the Image struct supports surfaces, we want to use textures, because they're
     * hardware accelerated. SDL will fall back to surfaces if it has to.
     * Our support of surfaces is an artifact from SDL1.
     */
    
    Image *result = init_Image(malloc(sizeof(Image)));;
    result->_texture = loadTextureFromFile(filename);
    SDL_QueryTexture(result->_texture, NULL, NULL, &(result->width), &(result->height));
        
    addImageToBatchIfBatching(result);        
    return result;
}

void deepCopy_Animation(Animation *to, Animation *from){
    to->numLoops = from->numLoops;
    
    to->loopLength = malloc(sizeof(int) * to->numLoops);
    to->frameStartTime = malloc(sizeof(int *) * to->numLoops);
    to->loopEndTime = malloc(sizeof(int *) * to->numLoops);
    to->repeatLoop = malloc(sizeof(int) * to->numLoops);
    to->spriteIndices = malloc(sizeof(int *) * to->numLoops);
    
    int i, j;
    for (i = 0; i < to->numLoops; i++){
        to->loopLength[i] = from->loopLength[i];
        
        to->spriteIndices[i] = malloc(sizeof(int) * to->loopLength[i]);
        to->frameStartTime[i] = malloc(sizeof(int) * to->loopLength[i]);
        
        to->repeatLoop[i] = from->repeatLoop[i];
        to->loopEndTime[i] = from->loopEndTime[i];
        for (j = 0; j < to->loopLength[i]; j++){
            to->spriteIndices[i][j] = from->spriteIndices[i][j];
            to->frameStartTime[i][j] = from->frameStartTime[i][j];
        }
    }    
    
    //reset the members tracking animation state
    to->currLoop = 0;
    to->currFrame = 0;
    to->milliPassed = 0;
}

Animation *shallowCopyAnimation(Animation *original){
    Animation *result = malloc(sizeof(Animation));
    (*result) = (*original);
    
    //reset the members tracking animation state
    result->currLoop = 0;
    result->currFrame = 0;
    result->milliPassed = 0;
    
    return result;
}

void startBatchingLoadedImages(){
    if (isBatching){
        LOG_WAR("Tried to start batching images when already batching");
    }
    
    isBatching = 1;
}

ImageAtlas *stopBatchingLoadedImages(){
    if (!isBatching){
        LOG_WAR("Tried to stop batching images when we weren't batching");
        return NULL;
    }
    
    //we have to do this now, otherwise our atlas images will get added to the list of things to batch
    isBatching = 0;
    
    int i, imageIndex, rowIndex;
    int foundImageWithSpace, foundRowWithSpace;
    int widthRemainingInRow;
    ImageRect dst;
    
    Image **newImages = NULL; //array of pointers
    int numNewImages = 0;
    int *numRows = NULL; //array [imageNumber]
    int **topUnusedPixelOfRow = NULL; //2d array, [image number][row number]
    int **leftUnusedPixelOfRow = NULL; //2d array [image number][row number]
    int **rowHeight = NULL; //2d array [atlas number][row number]
    int *remainingHeight = NULL; //array [imageNumber]
    
    //PIZZA - DO WE SORT?
    
    //our textures are PROBABLY all about the same size, so we just use a simple naive algorithm
    //we partition the texture into rows; for each texture, find the first row that is tall enough to fit the texture,
    //and has enough space on the end, then insert the texture there.  If no such row exists, add a new row or create a new texture
    //of course, if the images are sorted in reverse size order, this will fail spectacularly
    for (i = 0; i < numberOfImagesToBatch; i++){
        //skip anything that shares memory
        if (imagesToBatch[i]->_isShared){
            LOG_DEB("Image at %p not batched", imagesToBatch[i]);
            continue;
        }
        
        //try to add the image to an existing atlas
        foundImageWithSpace = 0;
        foundRowWithSpace = 0;
        for (imageIndex = 0; imageIndex < numNewImages; imageIndex++){
            for (rowIndex = 0; rowIndex < numRows[imageIndex]; rowIndex++){
                widthRemainingInRow = maxTextureWidth - leftUnusedPixelOfRow[imageIndex][rowIndex];
                if (imagesToBatch[i]->width <= widthRemainingInRow && imagesToBatch[i]->height <= rowHeight[imageIndex][rowIndex]){
                    foundRowWithSpace = 1;
                    foundImageWithSpace = 1;
                    break;
                }
            }
            
            //we use these loop indices below rather than store more variables, so ugly breaks from the loop it is
            if (foundRowWithSpace){
                break;
            
            //add a row if there is space in this image
            } else if (imagesToBatch[i]->height <= remainingHeight[imageIndex]){
                foundImageWithSpace = 1;
                break;
            }
        }
        
        //either add a new image and add our stuff there
        if (!foundRowWithSpace){
            //if we didn't find an image with space, create a new image, which will get a row added to it below
            if (!foundImageWithSpace){
                numNewImages++;
                imageIndex = numNewImages - 1;
                
                newImages = realloc(newImages, sizeof(Image *) * numNewImages);
                newImages[imageIndex] = createEmptyImage(maxTextureWidth, maxTextureHeight);
                
                numRows = realloc(numRows, sizeof(int) * numNewImages);
                numRows[imageIndex] = 0;
                
                topUnusedPixelOfRow = realloc(topUnusedPixelOfRow, sizeof(int *) * numNewImages);
                topUnusedPixelOfRow[imageIndex] = NULL; //NULL so realloc works below
                
                leftUnusedPixelOfRow = realloc(leftUnusedPixelOfRow, sizeof(int *) * numNewImages);
                leftUnusedPixelOfRow[imageIndex] = NULL; //NULL so realloc works below
                
                rowHeight = realloc(rowHeight, sizeof(int *) * numNewImages);
                rowHeight[imageIndex] = NULL; //NULL so realloc works below
                
                remainingHeight = realloc(remainingHeight, sizeof(int) * numNewImages);
                remainingHeight[imageIndex] = maxTextureHeight;
            }
            
            //add a row to the image
            numRows[imageIndex]++;
            rowIndex = numRows[imageIndex] - 1;
            
            topUnusedPixelOfRow[imageIndex] = realloc(topUnusedPixelOfRow[imageIndex], sizeof(int) * numRows[imageIndex]);
            topUnusedPixelOfRow[imageIndex][rowIndex] = maxTextureHeight - remainingHeight[imageIndex];
            
            leftUnusedPixelOfRow[imageIndex] = realloc(leftUnusedPixelOfRow[imageIndex], sizeof(int) * numRows[imageIndex]);
            leftUnusedPixelOfRow[imageIndex][rowIndex] = 0;
            
            rowHeight[imageIndex] = realloc(rowHeight[imageIndex], sizeof(int) * numRows[imageIndex]);
            rowHeight[imageIndex][rowIndex] = imagesToBatch[i]->height;
            
            remainingHeight[imageIndex] -= imagesToBatch[i]->height;
        }
        
        //take the chosen image and row and copy the image
        dst.x = leftUnusedPixelOfRow[imageIndex][rowIndex];
        dst.y = topUnusedPixelOfRow[imageIndex][rowIndex];
        dst.w = imagesToBatch[i]->width;
        dst.h = imagesToBatch[i]->height;
        drawImageToImage(imagesToBatch[i], newImages[imageIndex], NULL, &dst);
        
        //change the existing image and free its data
        SDL_DestroyTexture(imagesToBatch[i]->_texture);
        imagesToBatch[i]->_texture = newImages[imageIndex]->_texture;
        imagesToBatch[i]->_x = dst.x;
        imagesToBatch[i]->_y = dst.y;
        imagesToBatch[i]->_isShared = 1;
        
        //update row data
        leftUnusedPixelOfRow[imageIndex][rowIndex] += imagesToBatch[i]->width;
    }
    
    //free the list of pointers but not the pointers themselves, as they've just been updated
    free(imagesToBatch);
    imagesToBatch = NULL;
    numberOfImagesToBatch = 0;
    
    //free our data for positioning
    for (imageIndex = 0; imageIndex < numNewImages; imageIndex++){
        free(topUnusedPixelOfRow[imageIndex]);
        free(leftUnusedPixelOfRow[imageIndex]);
        free(rowHeight[imageIndex]);
    }
    free(numRows);
    free(topUnusedPixelOfRow);
    free(leftUnusedPixelOfRow);
    free(rowHeight);
    free(remainingHeight);
    
    //store our atlases
    ImageAtlas *result = init_ImageAtlas(malloc(sizeof(ImageAtlas)));
    result->images = newImages;
    result->numImages = numNewImages;
    
    return result;
}

void addImageToBatchIfBatching(Image *toBeBatched){
    if (isBatching){
        numberOfImagesToBatch++;
        imagesToBatch = realloc(imagesToBatch, sizeof(Image *) * numberOfImagesToBatch);
        imagesToBatch[numberOfImagesToBatch - 1] = toBeBatched;
    }
}

Animation *getNoAnimation(){
    Animation *result = init_Animation(malloc(sizeof(Animation)));
    
    result->numLoops = 1;
    
    result->loopLength = malloc(1 * sizeof(int));
    result->loopLength[0] = 1;
    
    result->frameStartTime = malloc(1 * sizeof(int *));
    result->frameStartTime[0] = malloc(1 * sizeof(int));
    result->frameStartTime[0][0] = 0;
    
    result->loopEndTime = malloc(1 * sizeof(int));
    result->loopEndTime[0] = 1;
    
    result->repeatLoop = malloc(1 * sizeof(int));
    result->repeatLoop[0] = 1;
    
    result->spriteIndices = malloc(1 * sizeof(int *));
    result->spriteIndices[0] = malloc(1 * sizeof(int));
    result->spriteIndices[0][0] = 0;
    
    return result;
}


/////////////////////////////////////////////////
// Animation Management
/////////////////////////////////////////////////
int updateAnimation(Animation *self, int delta){
    //return 1 on invalid input, 0 on success, 2 if the loop has ended
    if (delta < 0){
        LOG_WAR("Tried to update animation with a negative time change");
        return 1;
    }
    
    //increase our counter of time passed
    self->milliPassed += delta;
    
    //if we run over the total length of this loop, and the loop repeats, mod to restart the count
    int currLoop = self->currLoop;
    if (self->repeatLoop[currLoop] && self->milliPassed >= self->loopEndTime[currLoop]){
        self->milliPassed %= self->loopEndTime[currLoop];
    }
    
    //go through the frames for this loop until we get to the last one, or find the one that starts after our current time
    int i;
    for (i = 0; i < self->loopLength[currLoop]; i++){
        if (self->frameStartTime[currLoop][i] > self->milliPassed){
            break;
        }
    }
    self->currFrame = i-1;
    
    //if the loop doesn't repeat, then reset milliPassed
    if (!self->repeatLoop[currLoop] && self->milliPassed >= self->loopEndTime[currLoop]) {
        self->milliPassed = self->loopEndTime[currLoop];
        return 2;
    } else {
        return 0;
    }
}

void setAnimationLoop(Animation *self, int loop, int forceRestart){
    if (loop < 0 || loop >= self->numLoops){
        LOG_WAR("Tried to set an invalid animation loop");
        return;
    }
    
    //if a different loop has been provided, or you're forcing the loop to restart, set everything to 0
    if (self->currLoop != loop || forceRestart){
        self->currLoop = loop;
        self->currFrame = 0;
        self->milliPassed = 0;
    }
}


/////////////////////////////////////////////////
// Drawing
/////////////////////////////////////////////////
void drawImage(Image *image, int x, int y){
    SDL_Rect src, dest;
    
    src.x = image->_x;
    src.y = image->_y;
    src.w = image->width;
    src.h = image->height;
    
    dest.x = x;
    dest.y = y;
    dest.w = image->width;
    dest.h = image->height;

    if (SDL_RenderCopy(renderer, image->_texture, &src, &dest) != 0){
        LOG_ERR("Call to SDL_RenderCopy failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
}

void drawImageRotate(Image *image, float x, float y, float angle, int centerX, int centerY){
    SDL_Rect src, dest;
    
    src.x = image->_x;
    src.y = image->_y;
    src.w = image->width;
    src.h = image->height;
    
    dest.x = x;
    dest.y = y;
    dest.w = image->width;
    dest.h = image->height;
    
    SDL_Point center;
    center.x = centerX;
    center.y = centerY;

    if (SDL_RenderCopyEx(renderer, image->_texture, &src, &dest, angle, &center, SDL_FLIP_NONE) != 0){
    //if (SDL_RenderCopyEx(renderer, image->_texture, &src, &dest, angle, NULL, SDL_FLIP_NONE) != 0){
        LOG_ERR("Call to SDL_RenderCopy failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
}

void drawImageToImage(Image *src, Image *dst, ImageRect *srcRect, ImageRect *dstRect){
    //create SDL_Rects from ImageRects - PIZZA make internal method for this conversion
    SDL_Rect sR, dR;
    SDL_Rect *sRPtr, *dRPtr;
    if (srcRect != NULL){
        sR.x = srcRect->x + src->_x;
        sR.y = srcRect->y + src->_y;
        sR.w = srcRect->w;
        sR.h = srcRect->h;
    }
    if (dstRect != NULL){
        dR.x = dstRect->x + dst->_x;
        dR.y = dstRect->y + dst->_y;
        dR.w = dstRect->w;
        dR.h = dstRect->h;
    }
    sRPtr = (srcRect != NULL) ? &sR : NULL;
    dRPtr = (dstRect != NULL) ? &dR : NULL;
    
    //change the renderer to point to the texture in the destination image, then render the source image, then revert to the default render target
    if (SDL_SetRenderTarget(renderer, dst->_texture) != 0){
        LOG_ERR("Call to SDL_SetRenderTarget failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
    
    if (SDL_RenderCopy(renderer, src->_texture, sRPtr, dRPtr) != 0){
        LOG_ERR("Call to SDL_RenderCopy failed: %s", SDL_GetError());//PIZZA, need to verify can render to texture, check access flags
        displayErrorAndExit("Graphics error encountered");
    }
    
    if (SDL_SetRenderTarget(renderer, NULL) != 0){
        LOG_ERR("Call to SDL_SetRenderTarget failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
}

void drawImageSrcDst(Image *image, ImageRect *srcRect, ImageRect *dstRect){
    //create SDL_Rects from ImageRects - PIZZA make internal method for this conversion
    SDL_Rect sR, dR;
    SDL_Rect *sRPtr, *dRPtr;
    if (srcRect != NULL){
        sR.x = srcRect->x + image->_x;
        sR.y = srcRect->y + image->_y;
        sR.w = srcRect->w;
        sR.h = srcRect->h;
    }
    if (dstRect != NULL){
        dR.x = dstRect->x;
        dR.y = dstRect->y;
        dR.w = dstRect->w;
        dR.h = dstRect->h;
    }
    sRPtr = (srcRect != NULL) ? &sR : NULL;
    dRPtr = (dstRect != NULL) ? &dR : NULL;
    
    if (SDL_RenderCopy(renderer, image->_texture, sRPtr, dRPtr) != 0){
        LOG_ERR("Call to SDL_RenderCopy failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
}

void drawUnfilledRect(int x, int y, int w, int h, int r, int g, int b){
    /*
     * Set the color, draw the rectangle, revert the color to black
     */
    SDL_Rect temp = (SDL_Rect){ x, y, w, h };
    
    if (SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE) != 0){
        LOG_ERR("Call to SDL_SetRenderDrawColor failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
    
    if (SDL_RenderDrawRect(renderer, &temp) != 0){
        LOG_ERR("Call to SDL_RenderDrawRect failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
    
    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE) != 0){
        LOG_ERR("Call to SDL_SetRenderDrawColor failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
}

void drawFilledRect(int x, int y, int w, int h, int r, int g, int b){
    drawFilledRectA(x, y, w, h, r, g, b, SDL_ALPHA_OPAQUE);
}

void drawFilledRectA(int x, int y, int w, int h, int r, int g, int b, int a){
    /*
     * Set the color, draw the rectangle, revert the color to black
     */
    SDL_Rect temp = (SDL_Rect){ x, y, w, h };
    
    if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) != 0){
        LOG_ERR("Call to SDL_SetRenderDrawBlendMode failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
    
    if (SDL_SetRenderDrawColor(renderer, r, g, b, a) != 0){
        LOG_ERR("Call to SDL_SetRenderDrawColor failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
    
    if (SDL_RenderFillRect(renderer, &temp) != 0){
        LOG_ERR("Call to SDL_RenderFillRect failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
    
    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE) != 0){
        LOG_ERR("Call to SDL_SetRenderDrawColor failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
    
    if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE) != 0){
        LOG_ERR("Call to SDL_SetRenderDrawBlendMode failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
}

void drawLine(int x1, int y1, int x2, int y2, int r, int g, int b){
    /*
     * Set the color, draw the line, revert the color to black
     */
    if (SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE) != 0){
        LOG_ERR("Call to SDL_SetRenderDrawColor failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
    
    if (SDL_RenderDrawLine(renderer, x1, y1, x2, y2) != 0){
        LOG_ERR("Call to SDL_RenderDrawLine failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
    
    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE) != 0){
        LOG_ERR("Call to SDL_SetRenderDrawColor failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
}

void drawAnimation(Sprite *s, Animation *anim, int x, int y){
    /*
     * If the animation is null, just draw the image contained in the sprite
     * otherwise draw the animation
     */
    if (anim == NULL){
        drawImage(s->image, x, y);
    } else {
    
        //set the src/dst rectangles
        int spriteIndex = anim->spriteIndices[anim->currLoop][anim->currFrame];
        ImageRect src, dest;
        src.x = (spriteIndex % s->numFramesPerRow) * s->frameWidth;
        src.y = (spriteIndex / s->numFramesPerRow) * s->frameHeight;
        src.w = s->frameWidth;
        src.h = s->frameHeight;
        
        dest.x = x;
        dest.y = y;
        dest.w = s->frameWidth;
        dest.h = s->frameHeight;
        
        //draw
        drawImageSrcDst(s->image, &src, &dest);
    }
}


/////////////////////////////////////////////////
// Screen Management
/////////////////////////////////////////////////
void clearScreen(){
    if (SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE) != 0){
        LOG_ERR("Call to SDL_SetRenderDrawColor failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
        
    if (SDL_RenderClear(renderer) != 0){
        LOG_ERR("Call to SDL_RenderClear failed: %s", SDL_GetError());
        displayErrorAndExit("Graphics error encountered");
    }
}

void bufferToScreen(){
    SDL_RenderPresent(renderer); //returns no status, cannot check for failure
}

void setDrawScaling(int scaling){
    SDL_RenderSetScale(renderer, scaling, scaling);
}
