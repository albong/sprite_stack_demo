#ifndef INPUT_H
#define INPUT_H

/*
 * This code abstracts away SDL's input stuff.  Internally there are structs tracking if keys are being pressed or not,
 * but we want to be agnostic of how the keys are bound (and allow for rebinding), so access methods use enums.
 */


/////////////////////////////////////////////////
// Enums
/////////////////////////////////////////////////
typedef enum {
    UP_BUTTON, 
    DOWN_BUTTON, 
    LEFT_BUTTON, 
    RIGHT_BUTTON, 
    A_BUTTON, //weapon A
    B_BUTTON, //Weapon B
    X_BUTTON, //interact
    Y_BUTTON, //???
    START_BUTTON, //menu
    SELECT_BUTTON, //???
    ESCAPE_BUTTON, //exits game, should only be used for debugging
    NO_BUTTON //for when no button is assigned, will never be pressed
} Button;


/////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////
void initInput();
void getInput();
void consumeAllInput();
int checkInput(Button b); //checks if a button is being pressed and hasn't been consumed
int checkAndConsumeInput(Button b); //checks if a button is being pressed and marks it as consumed so that other things won't see the press until it is released and respressed

#endif
