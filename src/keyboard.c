#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "keyboard.h"
#include "input.h"

void set_player1_keyboard_input( PlayerInput *player, const uint8_t *keyboard_state ) {
    if (keyboard_state[SDL_SCANCODE_W]) {
        printf("1: W arrow key\n");
        player->joystick_pos = JOYSTICK_UP;
    }
    if (keyboard_state[SDL_SCANCODE_S]) {
        printf("1: S arrow key\n");
        player->joystick_pos = JOYSTICK_DOWN;
    }
    if (keyboard_state[SDL_SCANCODE_D]) {
        printf("1: D arrow key\n");
        player->move_x = 1.0;
    }
    if (keyboard_state[SDL_SCANCODE_A]) {
        printf("1: A arrow key\n");
        player->move_x = -1.0;
    }
    if (keyboard_state[SDL_SCANCODE_6]) {
        printf("1: attack (6)\n");
        player->attack_pressed = true;
    }
    if (keyboard_state[SDL_SCANCODE_5]) {
        printf("1: jump (5)\n");
        player->jump_pressed = true;
    }
}

void set_player2_keyboard_input( PlayerInput *player, const uint8_t *keyboard_state ) {
    if (keyboard_state[SDL_SCANCODE_UP]) {
        printf("2: up arrow key\n");
        player->joystick_pos = JOYSTICK_UP;
    }
    if (keyboard_state[SDL_SCANCODE_DOWN]) {
        printf("2: down arrow key\n");
        player->joystick_pos = JOYSTICK_DOWN;
    }
    if (keyboard_state[SDL_SCANCODE_RIGHT]) {
        printf("2: right arrow key\n");
        player->move_x = 1.0;
    }
    if (keyboard_state[SDL_SCANCODE_LEFT]) {
        printf("2: left arrow key\n");
        player->move_x = -1.0;
    }
    if (keyboard_state[SDL_SCANCODE_M]) {
        printf("2: attack (M)\n");
        player->attack_pressed = true;
    }
    if (keyboard_state[SDL_SCANCODE_N]) {
        printf("2: jump (N)\n");
        player->jump_pressed = true;
    }
}
