#include "input.h"
#include "SDL2/SDL.h"
#include <stdlib.h>

/*
    KEY LISTING
    -----------
    Start   - C
    Select  - X
    A       - S
    B       - D
    X       - W
    Y       - E
    Up      - UP
    Down    - DOWN
    Left    - LEFT
    Right   - RIGHT
    Exit    - ESC
*/

typedef struct Control {
    int up;
    int down;
    int left;
    int right;
    int a;
    int b;
    int x;
    int y;
    int start;
    int select;
    int escape;
} Control;

static Control input;
static Control inputRead;


void initInput(){
    input.up = 0;
    input.down = 0;
    input.left = 0;
    input.right = 0;
    input.a = 0;
    input.b = 0;
    input.x = 0;
    input.y = 0; 
    input.start = 0; 
    input.select = 0;
    input.escape = 0;
    
    inputRead.up = 0;
    inputRead.down = 0;
    inputRead.left = 0;
    inputRead.right = 0;
    inputRead.a = 0;
    inputRead.b = 0;
    inputRead.x = 0;
    inputRead.y = 0; 
    inputRead.start = 0; 
    inputRead.select = 0;
    inputRead.escape = 0;
}

void getInput(){
    SDL_Event event;
    
    while (SDL_PollEvent(&event)){
        switch (event.type){
            case SDL_QUIT:
                exit(0);
                break;
                
            case SDL_KEYDOWN:
                //switch here for different key down actions
                switch(event.key.keysym.sym){
    				case SDLK_UP:
						input.up = 1;
						break;
					
					case SDLK_DOWN:
						input.down = 1;
						break;
						
					case SDLK_LEFT:
						input.left = 1;
						break;
						
					case SDLK_RIGHT:
						input.right = 1;
						break;
					
					case SDLK_s:
						input.a = 1;
						break;
						
					case SDLK_d:
						input.b = 1;
						break;
					
					case SDLK_w:
						input.x = 1;
						break;
						
					case SDLK_e:
						input.y = 1;
						break;
					
					case SDLK_x:
						input.select = 1;
						break;
						
					case SDLK_c:
						input.start = 1;
						break;
					
					case SDLK_ESCAPE:
						// exit(0);
                        input.escape = 1;
						break;
						
                    default:
                        break;
                }
                break;
                
            case SDL_KEYUP:
                //switch statement here for different keyup actions
                switch(event.key.keysym.sym){
                    case SDLK_UP:
						input.up = 0;
						inputRead.up = 0;
						break;
					
					case SDLK_DOWN:
						input.down = 0;
						inputRead.down = 0;
						break;
						
					case SDLK_LEFT:
						input.left = 0;
						inputRead.left = 0;
						break;
						
					case SDLK_RIGHT:
						input.right = 0;
						inputRead.right = 0;
						break;
					
					case SDLK_s:
						input.a = 0;
						inputRead.a = 0;
						break;
						
					case SDLK_d:
						input.b = 0;
						inputRead.b = 0;
						break;
					
					case SDLK_w:
						input.x = 0;
						inputRead.x = 0;
						break;
						
					case SDLK_e:
						input.y = 0;
						inputRead.y = 0;
						break;
					
					case SDLK_x:
						input.select = 0;
						inputRead.select = 0;
						break;
						
					case SDLK_c:
						input.start = 0;
						inputRead.start = 0;
						break;
                        
                    case SDLK_ESCAPE:
						// exit(0);
                        input.escape = 0;
                        inputRead.escape = 0;
						break;
                    
                    default:
                        break;
                }
                break;
                
            default:
                break;
        }
    }
}

void consumeAllInput(){
    inputRead.up = (input.up) ? 1 : 0;
    inputRead.down = (input.down) ? 1 : 0;
    inputRead.left = (input.left) ? 1 : 0;
    inputRead.right = (input.right) ? 1 : 0;
    inputRead.a = (input.a) ? 1 : 0;
    inputRead.b = (input.b) ? 1 : 0;
    inputRead.x = (input.x) ? 1 : 0;
    inputRead.y = (input.y) ? 1 : 0;
    inputRead.start = (input.start) ? 1 : 0; 
    inputRead.select = (input.select) ? 1 : 0;
    inputRead.escape = (input.escape) ? 1 : 0;
}

int checkInput(Button b){
    int result = 0;

    switch (b){
        case UP_BUTTON:
            result = input.up && !inputRead.up;
            break;

        case DOWN_BUTTON:
            result = input.down && !inputRead.down;
            break;

        case LEFT_BUTTON:
            result = input.left && !inputRead.left;
            break;

        case RIGHT_BUTTON:
            result = input.right && !inputRead.right;
            break;

        case A_BUTTON:
            result = input.a && !inputRead.a;
            break;

        case B_BUTTON:
            result = input.b && !inputRead.b;
            break;

        case X_BUTTON:
            result = input.x && !inputRead.x;
            break;

        case Y_BUTTON:
            result = input.y && !inputRead.y;
            break;

        case START_BUTTON:
            result = input.start && !inputRead.start;
            break;

        case SELECT_BUTTON:
            result = input.select && !inputRead.select;
            break;

        case ESCAPE_BUTTON:
            result = input.escape && !inputRead.escape;
            break;

        default:
            result = 0;
            break;
    }

    return result;
}

int checkAndConsumeInput(Button b){
    int result = checkInput(b);

    switch (b){
        case UP_BUTTON:
            inputRead.up = (input.up) ? 1 : 0;
            break;

        case DOWN_BUTTON:
            inputRead.down = (input.down) ? 1 : 0;
            break;

        case LEFT_BUTTON:
            inputRead.left = (input.left) ? 1 : 0;
            break;

        case RIGHT_BUTTON:
            inputRead.right = (input.right) ? 1 : 0;
            break;

        case A_BUTTON:
            inputRead.a = (input.a) ? 1 : 0;
            break;

        case B_BUTTON:
            inputRead.b = (input.b) ? 1 : 0;
            break;

        case X_BUTTON:
            inputRead.x = (input.x) ? 1 : 0;
            break;

        case Y_BUTTON:
            inputRead.y = (input.y) ? 1 : 0;
            break;

        case START_BUTTON:
            inputRead.start = (input.start) ? 1 : 0;
            break;

        case SELECT_BUTTON:
            inputRead.select = (input.select) ? 1 : 0;
            break;

        case ESCAPE_BUTTON:
            inputRead.escape = (input.escape) ? 1 : 0;
            break;

        default:
            break;
    }

    return result;
}

