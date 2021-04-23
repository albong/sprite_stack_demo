#include "random.h"
#include "logging.h"
#include <stdint.h>
#include <inttypes.h>

static uint32_t state = 1234567;
static uint32_t xorshift32();

uint32_t xorshift32(){
    //not necessarily a great prng, but fast
    //implementation from wikipedia (from Marsaglia apparently): https://en.wikipedia.org/wiki/Xorshift
    uint32_t result = state;
    result ^= result << 13;
    result ^= result >> 17;
    result ^= result << 5;
    state = result;
    LOG_DEB("PRNG state updated to %" PRIu32, state);
    return result;
}

void seedPRNG(unsigned int seed){
    state = (uint32_t) seed;
    LOG_DEB("PRNG seeded with %u", seed);
}

int randomNumberLessThan(int upperBound){
    if (upperBound <= 0){
        LOG_WAR("Requested random number within non-positive range");
        return 0;
    }
    
    //many props to this dude: http://www.pcg-random.org/posts/bounded-rands.html
    //we went with biased integer multiplication for SPEED
    uint32_t x = xorshift32();
    uint64_t m = ((uint32_t)x) * ((uint64_t) upperBound);
        
    return (int)(m >> 32);
}
