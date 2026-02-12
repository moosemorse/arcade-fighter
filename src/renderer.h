#ifndef RENDERER_H
#define RENDERER_H

#include "game_types.h"

void renderer_init( void );
void renderer_set_player_size( double height, double width );
void renderer_begin_frame( void );
void renderer_draw_player( PlayerState player, double dt );
void renderer_end_frame( void );
void renderer_clean( void );
void renderer_draw_background( double dt );
bool SDL_event_handler( void );

#endif
