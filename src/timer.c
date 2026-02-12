#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL_timer.h>
#include "timer.h"

//NOTE: Must free
Timer timer_create( void ) {
    Timer t = malloc(sizeof(struct Timer));
    assert(t != NULL);
    t->_start_counter = 0;
    t->_paused_counter = 0;

    t->_started = false;
    t->_paused = false;

    return t;
}

void timer_start( Timer t ) {
    t->_started = true;
    t->_paused = false;

    t->_start_counter = SDL_GetPerformanceCounter();
    t->_paused_counter = 0;
}

void timer_reset( Timer t ) {
    t->_started = false;
    t->_paused = false;

    t->_start_counter = 0;
    t->_paused_counter = 0;
}

void timer_pause( Timer t ) {
    // Should only be able to pause timer if started
    if ( t->_started && !t->_paused ) {
        t->_paused = true;

        t->_paused_counter = SDL_GetPerformanceCounter() - t->_start_counter;
        t->_start_counter = 0;
    }
}

// When we pause want the relative time to still be x ticks away from current SDL_GetTicks time
void timer_unpause( Timer t ) {
    if( t->_started && t->_paused ) {
        t->_paused = false;

        t->_start_counter = SDL_GetPerformanceCounter() - t->_paused_counter;
        t->_paused_counter = 0;
    }
}

// Returns relative time of when started timer in seconds - or when timer was paused
double timer_get_seconds( Timer t ) {
    double time = 0.0;

    if ( t->_started ) {
        uint64_t elapsed = (t->_paused) ? t->_paused_counter : SDL_GetPerformanceCounter() - t->_start_counter;
        time = (double) elapsed / (double) SDL_GetPerformanceFrequency();
   }
    return time;
}

void timer_free ( Timer t ) {
    free(t);
}
