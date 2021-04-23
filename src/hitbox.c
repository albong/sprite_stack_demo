#include "hitbox.h"
#include "logging.h"
#include <stdlib.h>

//oh lordy get rid of this
#define square(x) ((x)*(x))

Hitbox *init_Hitbox(Hitbox *self){
    self->boundingBox.x = 0;
    self->boundingBox.y = 0;
    self->boundingBox.w = 0;
    self->boundingBox.h = 0;
    
    self->numRectangles = 0;
    self->numCircle = 0;
    self->rects = NULL;
    self->circles = NULL;
    
    return self;
}

Hitboxes *init_Hitboxes(Hitboxes *self){
    self->numLoops = 0;
    self->loopLength = NULL;
    self->shapes = NULL;
    
    return self;
}

void term_Hitbox(Hitbox *self){
    if (self == NULL){
        LOG_WAR("Tried to terminate NULL hitbox");
        return;
    }
    
    self->boundingBox.x = 0;
    self->boundingBox.y = 0;
    self->boundingBox.w = 0;
    self->boundingBox.h = 0;
    
    free(self->rects);
    free(self->circles);
    
    self->numRectangles = 0;
    self->numCircle = 0;
    self->rects = NULL;
    self->circles = NULL;
}

void term_Hitboxes(Hitboxes *self){
    if (self == NULL){
        LOG_WAR("Tried to terminate NULL hitboxes");
        return;
    }
    
    int i, j;
    for (i = 0; i < self->numLoops; i++){
        for (j = 0; j < self->loopLength[i]; j++){
            term_Hitbox(self->shapes[i] + j);
        }
        free(self->shapes[i]);
    }
    
    free(self->shapes);
    self->shapes = NULL;
    
    free(self->loopLength);
    self->loopLength = NULL;
    self->numLoops = 0;
}

void deepCopy_Hitboxes(Hitboxes *to, Hitboxes *from){
    to->numLoops = from->numLoops;
    to->loopLength = malloc(sizeof(int) * to->numLoops);
    to->shapes = malloc(sizeof(Hitbox *) * to->numLoops);
    
    int i, j;
    for (i = 0; i < to->numLoops; i++){
        to->loopLength[i] = from->loopLength[i];
        
        to->shapes[i] = malloc(sizeof(Hitbox) * to->loopLength[i]);
        
        for (j = 0; j < to->loopLength[i]; j++){
            deepCopy_Hitbox(&(to->shapes[i][j]), &(from->shapes[i][j]));
        }
    }
}

void deepCopy_Hitbox(Hitbox *to, Hitbox *from){
    int i;
    
    (*to) = (*from);
    to->rects = malloc(sizeof(CollisionRectangle) * to->numRectangles);
    to->circles = malloc(sizeof(CollisionCircle) * to->numCircle);
    
    for (i = 0; i < to->numRectangles; i++){
        to->rects[i] = from->rects[i];
    }
    for (i = 0; i < to->numCircle; i++){
        to->circles[i] = from->circles[i];
    }
}


/////////////////////////////////////////////////
// Access
/////////////////////////////////////////////////
void computeBoundingBox(Hitbox *self){
    int k;
    float minX, maxX, minY, maxY;
    
    minX = 0;
    minY = 0;
    maxX = 0;
    maxY = 0;
    
    //iterate over each rectangle in the frame
    for (k = 0; k < self->numRectangles; k++){
        //compute bounding box - use the first rectangle, then compare against the later ones
        if (k == 0){
            minX = self->rects[k].x;
            minY = self->rects[k].y;
            maxX = self->rects[k].x + self->rects[k].w;
            maxY = self->rects[k].y + self->rects[k].h;
        } else {
            minX = (self->rects[k].x < minX) ? self->rects[k].x : minX;
            minY = (self->rects[k].y < minY) ? self->rects[k].y : minY;
            maxX = ((self->rects[k].x + self->rects[k].w) > maxX) ? (self->rects[k].x + self->rects[k].w) : maxX;
            maxY = ((self->rects[k].y + self->rects[k].h) > maxY) ? (self->rects[k].y + self->rects[k].h) : maxY;
        }
    }
    
    self->boundingBox.x = minX;
    self->boundingBox.y = minY;
    self->boundingBox.w = maxX - minX;
    self->boundingBox.h = maxY - minY;
}


/////////////////////////////////////////////////
// Collision checkers
/////////////////////////////////////////////////
int simpleRectangleCollide(CollisionRectangle A, CollisionRectangle B){
    if (A.x+A.w <= B.x || B.x+B.w <= A.x || A.y+A.h <= B.y || B.y+B.h <= A.y){
        return 0;
    } else {
        return 1;
    }
}

int detailedRectangleCollide(CollisionRectangle A, CollisionRectangle B, CollisionDetail *detailedResult){
    //taken from the end of:
    // https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331
    
    //is this actually faster?  who knows
    float AHalfW = A.w / 2.0;
    float AHalfH = A.h / 2.0;
    float BHalfW = B.w / 2.0;
    float BHalfH = B.h / 2.0;
    
    //vector from center of A to center of B - we actually only need the size of these
    float nVectorX = (B.x + BHalfW) - (A.x + AHalfW);
    float nVectorY = (B.y + BHalfH) - (A.y + AHalfH);
    
    //compute overlap - ternaries are poor mans absolute value
    float xOverlap = AHalfW + BHalfW - ((nVectorX >= 0) ? nVectorX : -nVectorX);
    float yOverlap = AHalfH + BHalfH - ((nVectorY >= 0) ? nVectorY : -nVectorY);
    if (xOverlap > 0 && yOverlap > 0){
        if (xOverlap < yOverlap){
            //point to B since n points from A to B
            if (nVectorX < 0){
                detailedResult->normalX = -1;
                detailedResult->normalY = 0;
            } else {
                detailedResult->normalX = 1;
                detailedResult->normalY = 0;
            }
            
            detailedResult->penetration = xOverlap;
        } else {
            //point to B since n points from A to B
            if (nVectorY < 0){
                detailedResult->normalX = 0;
                detailedResult->normalY = -1;
            } else {
                detailedResult->normalX = 0;
                detailedResult->normalY = 1;
            }
            
            detailedResult->penetration = yOverlap;
        }
        
        return 1;
    } else {//no collision
        return 0;
    }
}

// OLD, DON'T USE
/////////////////////////////////////////////////
int rectangleCollide(CollisionRectangle r1, CollisionRectangle r2){
    ///NOTE: sometimes you can get weird numbers returned, negatives and such; I think this occurs when rectangles are on top of each other.
    
    //1 = left, 2, = right, 3 = up, 4 = down - these are for which side of r1 is hit by r2
    int result = 0;
    
    //fast check before we check all sides
    if (r1.x+r1.w <= r2.x || r2.x+r2.w <= r1.x || r1.y+r1.h <= r2.y || r2.y+r2.h <= r1.y){
        return 0;
    }
    
    //if we have a collision, then determine the side
    if (r2.x <= r1.x && r1.x <= r2.x+r2.w){
        result |= 1;
    }
    if (r2.x <= r1.x+r1.w && r1.x+r1.w <= r2.x+r2.w){
        result |= 2;
    }
    if (r2.y <= r1.y && r1.y <= r2.y+r2.h){
        result |= 4;
    }
    if (r2.y <= r1.y+r1.h && r1.y+r1.h <= r2.y+r2.h){
        result |= 8;
    }
    if (!result){
        result = -1;
    }

    return result;
}

int circleCollide(CollisionCircle c1, CollisionCircle c2){
    if (square(c2.cx-c1.cx) + square(c2.cy-c1.cy) < square(c1.r+c2.r)){
        return 1;
    } else {
        return 0;
    }
}

int rectangleCircleCollide(CollisionRectangle r, CollisionCircle c){
    //check if edge of rectangle is inside of circle
    //then check if a corner of rectangle is inside of circle
    if ((r.x < c.cx && c.cx < r.x+r.w && square(c.cy-r.y) < square(c.r)) ||
            (r.x < c.cx && c.cx < r.x+r.w && square(c.cy-r.y-r.h) < square(c.r)) ||
            (r.y < c.cy && c.cy < r.y+r.h && square(c.cx-r.x) < square(c.r)) ||
            (r.y < c.cy && c.cy < r.y+r.h && square(c.cx-r.x-r.w) < square(c.r)) ||
            (square(r.x-c.cx) + square(r.y-c.cy) < square(c.r)) ||
            (square(r.x-c.cx) + square(r.y+r.h-c.cy) < square(c.r)) ||
            (square(r.x+r.w-c.cx) + square(r.y-c.cy) < square(c.r)) ||
            (square(r.x+r.w-c.cx) + square(r.y+r.h-c.cy) < square(c.r))){
        return 1;
    } else {
        return 0;
    }
}
