#include "debug_drawer.h"
#include "graphics.h"
//#include "../src/constants.h"
#include "configuration.h"
#include "logging.h"
#include "font.h"
#include <stdio.h>

void debugComputeAndDisplayFramerate(unsigned delta){
    if (!DEBUG_DRAW_FRAMERATE){
        return;
    }
    
    //this is like, the laziest way to do this
    //its more stable to count frames in a second and display that, but it won't respond to spikes that way
    unsigned rate;
    char rateString[5];
    if (delta == 0){
        rate = 0;
        sprintf(rateString, "-1");
    } else {
        rate = 1000/delta;
        sprintf(rateString, "%u", rate);
    }
    
    
    //draw the framerate
    Font* currentFont = getCurrentTextFont();
    FontCharacter *fc;
    ImageRect src, dst;
    int cursorX = 0;
    int cursorY = 0;
    int i;
    for (i = 0; i < 5; i++){
        if (rateString[i] == '\0'){
            break;
        }
        
        fc = findCharacter(currentFont, rateString[i]);
        if (fc != NULL){
            //where is the character in the font sheet
            src.x = fc->x;
            src.y = fc->y;
            src.w = fc->width;
            src.h = fc->height;
            
            //adjust the destination accordingly
            dst.x = cursorX + fc->xOffset;
            dst.y = cursorY + fc->yOffset;
            dst.w = fc->width;
            dst.h = fc->height;
            
            //draw
            drawImageSrcDst(currentFont->fontSheets[fc->sheetNum], &src, &dst);
            
            cursorX += fc->xAdvance;
        } else {
            drawFilledRect(cursorX, cursorY, 1, currentFont->lineHeight, 255, 0, 0);
            cursorX += 1;
            LOG_ERR("Somehow couldn't draw a letter of the framerate!");
        }
    }
}
