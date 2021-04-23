#ifndef CONSTANTS_H
#define CONSTANTS_H

/*
 * Contains all defined constants used in the game.
 * 
 * REMEMBER TO USE PARENTHESES FOR COMPUTED CONSTANTS
 */

//
extern const int ENGINE_VERSION_MAJOR;
extern const int ENGINE_VERSION_MINOR;
extern const int ENGINE_VERSION_PATCH;

extern const int IS_DEVELOPMENT_VERSION;

extern const int GAME_VERSION_MAJOR;
extern const int GAME_VERSION_MINOR;
extern const int GAME_VERSION_PATCH;
extern const char *GAME_NAME;

//30fps ~33
//60fps ~16
//240fps ~4
//used to limit the logic/physics to a fixed timestep
#define FRAME_TIMESTEP_MILLISECONDS 33
//prevents the spiral of death with logic in the game loop, will run 5 steps of logic before forcing a draw
#define MAX_FRAMES_BEFORE_FORCE_DRAW 5

//room constants
//in theory the compiler will make the derived constants actual constants, but best to compute the numbers ourselves and not leave it to chance
#define ROOM_WIDTH_TILES 11
#define ROOM_HEIGHT_TILES 9
#define TILE_SIZE_PIXELS 16
#define ROOM_WIDTH_PIXELS 176
#define ROOM_HEIGHT_PIXELS 176
#define ROOM_CENTER_X_PIXELS 88
#define ROOM_CENTER_Y_PIXELS 72
#define ROOM_EDGE_TOLERANCE_PIXELS 100

#define DRAW_LAYERED_AREAS 1

/*
 * The engine renders at two scales - the game scale is small, while the ui/text/etc render
 * at a higher resolution for fidelity. The game operates logically at its small resolution
 * and then it gets upscaled to the actual window resolution.
 * In this way we work with two pre-determined resolutions - one for gameplay, and one for ui.
 * The final rendered resolution may be different yet, as we may be asking SDL to magically scale
 * up.
 * Here SCREEN_WIDTH and SCREEN_HEIGHT are the game resolution, and WINDOW_WIDTH and WINDOW_HEIGHT
 * are the ui resolution. The render scale is also given.
 * 
 * These should be 176x144 and 880x720
 */
#define SCREEN_WIDTH (ROOM_WIDTH_PIXELS)
#define SCREEN_HEIGHT (ROOM_HEIGHT_PIXELS)
#define RENDER_SCALE_MULTIPLE 5
#define WINDOW_WIDTH (SCREEN_WIDTH * RENDER_SCALE_MULTIPLE)
#define WINDOW_HEIGHT (SCREEN_HEIGHT * RENDER_SCALE_MULTIPLE)

extern const int MAX_TEXTURE_WIDTH;
extern const int MAX_TEXTURE_HEIGHT;

#define SQRT_2 1.4142135623730950
#define SQRT_2_RECIPROCAL 0.7071067811865475

#define ROOM_TRANSITION_DURATION_MILLISECONDS 550.0
extern const float ROOM_TRANSITION_SPEED_HORIZONTAL;
extern const float ROOM_TRANSITION_SPEED_VERTICAL;
#define MILLI_PER_SCREEN_WIPE 750.0
#define MILLI_HOLD_SCREEN_WIPE 75.0

#define ROOM_LEFT 0
#define ROOM_RIGHT 1
#define ROOM_UP 2
#define ROOM_DOWN 3

#define HITSTUN_FLASH_MILLI 75

#define LOAD_ICON_ENTITY_ID 4
#define MENU_ANIMATION_ID 12

#define FILENAME_BUFFER_SIZE 80

#define LINEFEED_UTF8_ID 10
#define SPACE_UTF8_ID 32
#define DASH_UTF8_ID 45

//multiple of CHAR_BIT please
#define NUM_FLAGS 4000

#define NUM_SOUND_CHANNELS 100

//defined so that we can have an array of this size, cannot use const
#define FRAME_STACK_SIZE 15

#endif
