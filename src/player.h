#ifndef PLAYER_H
#define PLAYER_H
#include <stdbool.h>
#include "game_types.h"

typedef struct PlayerInput PlayerInput;

extern void player_init( PlayerState player, PlayerId id );
extern void player_update( PlayerState player, PlayerInput input, double dt );
extern void player_set_death_state( PlayerState player );
extern void player_receive_dive_kick( PlayerState receiver, PlayerState attacker );
extern void player_end_dive_kick( PlayerState player );
extern void print_vector_2d( Vector_2D v );
extern void print_player( PlayerState player );

#endif
