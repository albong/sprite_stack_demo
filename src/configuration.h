#ifndef CONFIGURATION_H
#define CONFIGURATION_H

/*
 * Contains settings that are loaded from file and should be treated as constants, even though they're not.
 * 
 * This is not for user settings (language/resolution), but rather for runtime configurations about the engine.
 * 
 * By default, all debug options are 0 to be turned off.
 */


/////////////////////////////////////////////////
// "Constants"
/////////////////////////////////////////////////
//booleans, 0 or 1
//makes it easy to turn on or off debug settings
extern int DEBUG_DRAW_BOUNDING_BOXES;
extern int DEBUG_DRAW_ONLY_BOUNDING_BOXES;
extern int DEBUG_DRAW_ATTACK_HITBOX;
extern int DEBUG_DRAW_HURT_HITBOX;
extern int DEBUG_DRAW_INTERACT_HITBOX;
extern int DEBUG_DRAW_PHYSICAL_HITBOX;
extern int DEBUG_DRAW_WALL_HITBOX;
extern int DEBUG_DRAW_FRAMERATE;

//booleans, 0 or 1
//some startup settings for debugging
extern int DEBUG_SKIP_INITIAL_TITLE_SCREEN;
extern int DEBUG_USE_DEBUG_NEWGAME_FILE;


/////////////////////////////////////////////////
// Loading
/////////////////////////////////////////////////
void loadConfiguration();

#endif
