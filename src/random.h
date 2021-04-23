#ifndef RANDOM_H
#define RANDOM_H

/*
 * Used to generate random numbers.  Doesn't necessarily use the best algorithms under the hood, but they are fast af
 */

//Seed whatever PRNG is used
void seedPRNG(unsigned int seed);

//non-inclusively get a random number in [0, upperBound).  Returns int, but will be at least 0
int randomNumberLessThan(int upperBound);

#endif
