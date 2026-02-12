#ifndef INPUT_H
#define INPUT_H

#include "game_types.h"

typedef enum {
	JOYSTICK_UP, 
	JOYSTICK_DOWN, 
	JOYSTICK_MID
} JoystickPos; 

typedef struct PlayerInput { 
    double move_x; // -1.0 to 1.0 (left to right)
    JoystickPos joystick_pos; // direction of joystick
    bool attack_pressed; // attack button pressed
    bool jump_pressed; // jump button pressed
} PlayerInput; 

extern PlayerInput input_get( PlayerId player_id ); 

#endif

