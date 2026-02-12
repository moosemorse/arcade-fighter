#ifndef GAMETYPES_H
#define GAMETYPES_H

#include <stdbool.h>

#define SCREEN_SIZE_X 1280
#define SCREEN_SIZE_Y 720

typedef struct PlayerInternals *PlayerInternals;

typedef enum {   
	PLAYER_1, 
	PLAYER_2 
} PlayerId;

typedef enum {
    LOW,
    MIDDLE,
    HIGH
} Stance;

typedef struct {
    double x;
    double y;
} Vector_2D;

typedef struct {
    Vector_2D top_left;    // Coordinates of top left of box
    double width;          // Box width
    double height;         // Box height
    bool enabled;         
} Box;

typedef struct {
    Box hitbox;
    Vector_2D pos; // Center of sword
    PlayerId player; //If throw sword should know owner
    bool thrown;
    struct {
        double attack_timer;
        double attack_delay;
    } internal;
} Sword;


typedef struct PlayerState{
    PlayerId id;
    double time_in_anim;
    Vector_2D pos;  //Center of player
    Vector_2D vel;
    Stance stance;
    // bool is_armed;
    bool is_jumping;
    bool is_attacking;
    bool is_dead;
    bool is_right_facing;
    bool in_stunned_state; //Stunned sprite
    bool is_crouching;
    Box hitbox; //Punching or for dive kick
    Box hurtbox;
    Sword sword;
    struct {
        double jump_delay;
        double stance_delay;
        bool is_stunned; // Player cannot act/move
        double stunned_duration;
    } internal; 
} *PlayerState;

#endif
