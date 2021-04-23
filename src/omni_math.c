#include "omni_math.h"
#include <math.h>


void unitDirectionVector(float fromX, float fromY, float toX, float toY, float *resultX, float *resultY){
    float rX = toX - fromX;
    float rY = toY - fromY;
    float magnitude = sqrtf((rX*rX) + (rY*rY));
    *resultX = rX / magnitude;
    *resultY = rY / magnitude;
}

float distanceSquared(float x1, float y1, float x2, float y2){
    float xDiff = x2-x1;
    float yDiff = y2-y1;
    return (xDiff * xDiff) + (yDiff * yDiff);
}
