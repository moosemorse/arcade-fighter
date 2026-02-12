#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

typedef struct PlayerInput PlayerInput;

extern void set_player1_keyboard_input( PlayerInput * player, const uint8_t * keyboard_state );
extern void set_player2_keyboard_input( PlayerInput * player, const uint8_t * keyboard_state );

#endif
