#include <stdio.h>
//#include <pigpio.h>
#include "input.h"
#include "game_types.h"

static PlayerInput player1_input( void ); 
static PlayerInput player2_input( void ); 

PlayerInput input_get( PlayerId player_id ) 
{
    PlayerInput input; 
    switch (player_id) 
    { 
        case PLAYER_1: 
            input = player1_input(); 
            break;
        case PLAYER_2: 
            input = player2_input(); 
            break; 
        default: 
            fprintf(stderr, "Invalid playerId in input_get()");  
    }
    return input; 
}

static PlayerInput player1_input( void ) 
{  
    PlayerInput input;
    input.move_x = 0;
    input.joystick_pos = JOYSTICK_MID;
    input.attack_pressed = false;
    input.jump_pressed = false;
    return input;
}

static PlayerInput player2_input( void ) 
{ 
    return player1_input();
} 
