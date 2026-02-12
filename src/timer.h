#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

/* Timer that can start, stop and pause */
// All values in the struct are internal values - DO NOT USE THEM DIRECTLY unless you know what you are doing
// Instead use the timer by calling functions below e.g. timer_get_seconds()
typedef struct Timer {
    uint64_t _start_counter;
    uint64_t _paused_counter;
    bool _started;
    bool _paused;
} *Timer;

extern Timer timer_create( void );
extern void timer_start( Timer t );
extern void timer_reset( Timer t );
extern void timer_pause( Timer t );
extern void timer_unpause( Timer t );
extern double timer_get_seconds( Timer t );
extern void timer_free( Timer t );

#endif
