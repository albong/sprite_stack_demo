#ifndef FRAMES_H
#define FRAMES_H

/*
 * The uses a stack of frames to execute drawing and logic in a meaningful order, and also to segregate unrelated
 * sections of engine logic.  You could also think of frames as states that the engine is in, or maybe call them panes,
 * like panes of glass on top of each other.
 * 
 * For example, when you are playing the game, and you push the button to display the menu, the gameFrame
 * pushes the menuFrame onto the stack.  The menuFrame's logic is now executed while the gameFrame's is not,
 * but the gameFrame can still be drawn underneath of the menuFrame.  When you exit the menu, the menuFrame
 * is popped from the stack, and the logic for the gameFrame is resumed.
 * 
 * At the bottom of the stack is the frame for the title screen, starting a game pushes a loading screen
 * frame onto the stack which then pushes a game frame onto the stack when ready.  When you change areas,
 * the game frame pops off the top and control returns to the loading frame.
 * 
 * The code in frames.c contains some of the high-level logic for each frame and manages switching frames.
 * This also contains the main game loop, which executes each freme abstractly.
 */


/////////////////////////////////////////////////
// Loading/Unloading
/////////////////////////////////////////////////
void initFrames();
void termFrames();


/////////////////////////////////////////////////
// Game Logic
/////////////////////////////////////////////////
void gameLoop();


#endif
