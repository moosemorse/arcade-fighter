#ifndef SWORD_H
#define SWORD_H

#include "game_types.h"

Sword sword_init( PlayerState player ); 
void move_sword_to_player( Sword *sword, Vector_2D player_pos, bool right_facing, Stance stance ); 
void sword_update_internal_state( Sword *sword, bool is_attacking, double dt );
bool sword_can_attack( Sword *sword );
void sword_begin_melee_attack( Sword *sword, Stance stance );
void sword_update_attack_hitbox( PlayerState player );
bool sword_check_attack_over( Sword *sword, Stance stance );
int sword_get_frame( PlayerState player );

#endif
