#ifndef HITBOX_H
#define HITBOX_H

/*
 * The data types used for collisions.  Not with the collision code since I wanted as much of the
 * code as possible to be ignorant of the collision code.
 * 
 * This is kinda poorly named I guess, since a "hitbox" is actually a particular thing, but its too late.
 * 
 * The animation and hitbox logics are pretty tightly intertwined, both from how data files are
 * named and internally how hitboxes are processed.  Essentially, each frame of animation
 * gets its own hitbox, so the list of hitboxes matches up to the list of sprite frames, and the
 * animation saying to play frame 5 means that the sprite frame 5 and the hitbox at index 5 will be used.
 * 
 * Physicalboxes are physical (ie running into walls or etc), hitboxes damage when they collide with hurtboxes, and interactboxes are
 * for when the player wants to inspect something.
 */
 
/////////////////////////////////////////////////
// Enums
/////////////////////////////////////////////////
//This is used in the collision code to simplify access to the hitboxes
typedef enum HitboxType {
    ATTACK_HT, 
    HURT_HT, 
    PHYSICAL_HT, 
    INTERACT_HT
} HitboxType; 


/////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////
typedef struct CollisionRectangle {
    float x, y, w, h;
} CollisionRectangle;

//not actually used anywhere
typedef struct CollisionCircle {
    float cx, cy, r;
} CollisionCircle;

//A hitbox contans a number of rectangles and circles, and corresponds to a single frame of animation
//So really there are multiple "boxes" per hitbox
typedef struct Hitbox{
    CollisionRectangle boundingBox;
    int numRectangles;
    int numCircle;
    //arrays
    CollisionRectangle *rects;
    CollisionCircle *circles;
} Hitbox;

//A series of "hitboxes" each corresponding to a frame of animation.  Probably misnamed.
typedef struct Hitboxes {
    //this does end up duplicating data in the animation struct, but we need it to free the hitboxes
    //please don't look at these if possible
    int numLoops;
    int *loopLength;
    //array of arrays, first index is loop number, second index is frame number
    //how to access these is tied to the matching animation
    Hitbox **shapes;
} Hitboxes;

//used to store the result of a collision
typedef struct CollisionDetail {
    //direction of the overlap - unclear if its supposed to be a unit vector, but for rectangle collisions it will be
    float normalX;
    float normalY;
    //depth of the overlap
    float penetration;
} CollisionDetail;


/////////////////////////////////////////////////
// Loading/Unloading
/////////////////////////////////////////////////
Hitbox *init_Hitbox(Hitbox *self);
Hitboxes *init_Hitboxes(Hitboxes *self);
void term_Hitbox(Hitbox *self);
void term_Hitboxes(Hitboxes *self);
void deepCopy_Hitbox(Hitbox *to, Hitbox *from);
void deepCopy_Hitboxes(Hitboxes *to, Hitboxes *from);


/////////////////////////////////////////////////
// Access
/////////////////////////////////////////////////
void computeBoundingBox(Hitbox *self);


/////////////////////////////////////////////////
// Collision checkers
/////////////////////////////////////////////////
int simpleRectangleCollide(CollisionRectangle A, CollisionRectangle B);
int detailedRectangleCollide(CollisionRectangle A, CollisionRectangle B, CollisionDetail *detailedResult);


#endif
