#include <stdio.h>
#include "sword.h"
#include "game_types.h"

#define MAX_ATTACK_FRAMES   4
#define STANCE_NOS          3
#define STANCE_LOW_OFFSET   40.0
#define STANCE_HIGH_OFFSET   -27.5
#define STANCE_MIDDLE_OFFSET -13.0
#define STANCE_ANGLE_PENALTY 10.0

/* Sword constants */
#define SWORD_LENGTH 55.0    
#define SWORD_WIDTH 10.0

#define LOW_ATTACK_DURATION 1.1
#define LOW_ATTACK_DELAY 0.4

#define MID_ATTACK_DURATION 0.4
#define MID_ATTACK_DELAY 0.1

#define HIGH_ATTACK_DURATION 1.1
#define HIGH_ATTACK_DELAY 0.2


typedef struct {
    const double attack_frames[MAX_ATTACK_FRAMES]; // Array of attack frames
    double attack_over;
    Stance stance;
} AttackData;

static const AttackData attack_datas[] = {
    {
        {
            0.1,
            0.1,
            0.1,
            0.1
        },
        0.1,
        MIDDLE
    },
    {
        {
            0.2,
            0.3,
            0.4,
            0.2
        },
        0.4,
        LOW
    },
    {
        {
            0.6,
            0.1,
            0.3,
            0.1
        },
        0.2,
        HIGH
    }
};


typedef struct {
    Box frames[MAX_ATTACK_FRAMES];
    Stance stance;
} HitboxTuple;

// This stores the hitboxes (relative to player pos) of each frame of each stance
static const HitboxTuple attack_frames[] = {
    {
        .stance = HIGH,
        .frames = {
            {
                .top_left = {
                    .x = -100,
                    .y = -80
                },
                .height = 80,
                .width = 10,
                .enabled = true
            },
            {
                .top_left = {
                    .x = -20,
                    .y = -80
                },
                .height = 100,
                .width = 10,
                .enabled = true
            },
            {
                .top_left = {
                    .x = -40,
                    .y = -100
                },
                .height = 170,
                .width = 200,
                .enabled = true
            },
            {
                .enabled = false
            }
        }
    },
    {
        .stance = MIDDLE,
        .frames = {
            {
                .top_left = {
                    .x = -30,
                    .y = 60
                },
                .height = 20,
                .width = 110,
                .enabled = true
            },
            {
                .enabled = false
            },
            {
                .top_left = {
                    .x = -30,
                    .y = 60
                },
                .height = 20,
                .width = 170,
                .enabled = true
            },
            {
                .top_left = {
                    .x = -30,
                    .y = 60
                },
                .height = 20,
                .width = 110
            }
        }
    },
    {
        .stance = LOW,
        .frames = {
            {
                .top_left = {
                    .x = 80,
                    .y = 60
                },
                .height = 20,
                .width = 40,
                .enabled = true
            },
            {
                .enabled = false
            },
            {
                .top_left = {
                    .x = -150,
                    .y = 60
                },
                .height = 50,
                .width = 250,
                .enabled = true
            },
            {
                .top_left = {
                    .x = -180,
                    .y = 60
                },
                .height = 50,
                .width = 80,
                .enabled = true
            },
        }
    }
};

void move_sword_to_player( Sword *sword, Vector_2D player_pos, bool right_facing, Stance stance ); 
static void sword_update_hitbox( Sword *sword );

Sword sword_init( PlayerState player ) {
    Sword sword;
    
    sword.hitbox.width = SWORD_LENGTH;
    sword.hitbox.height = SWORD_WIDTH;
    sword.hitbox.enabled = false;
    
    sword.player = player->id;
    sword.thrown = false;
    
    move_sword_to_player(&sword, player->pos, player->is_right_facing, player->stance);

    sword.internal.attack_timer = 0;
    sword.internal.attack_delay = 0;

    return sword;
}

void move_sword_to_player( Sword *sword, Vector_2D player_pos, bool right_facing, Stance stance ) {
    int scale_factor = (right_facing) ? 1 : -1;
    // Center of player + half of sword (so sword pos starts at middle of player - as where its held)
    sword->pos.x = player_pos.x + scale_factor * sword->hitbox.width / 2.0;
    //TODO() depending on stance should be at different height
    sword->pos.y = player_pos.y;
    sword_update_hitbox(sword);
}

void sword_update_internal_state( Sword *sword, bool is_attacking, double dt ) 
{
    if( sword->internal.attack_delay > 0.0 )
    {
        sword->internal.attack_delay -= dt;
        if( sword->internal.attack_delay < 0.0 )
        {
            sword->internal.attack_delay = 0.0;
        }
    }

    if (is_attacking)
    {
        sword->internal.attack_timer += dt;
    }
}

bool sword_can_attack( Sword *sword )
{
    return sword->internal.attack_delay <= 0.0;
}

void sword_begin_melee_attack(Sword *sword, Stance stance)
{
    sword->hitbox.enabled = true;
    sword->internal.attack_timer = 0;
}

void sword_update_attack_hitbox( PlayerState player )
{
    int scale_factor = (player->is_right_facing) ? 1 : -1;
    if (player->is_attacking && !player->is_jumping)
    {
        for (int i = 0; i < STANCE_NOS; i ++)
        {
            if (player->stance == attack_frames[i].stance)
            {
                Sword *sword = &(player->sword);
                Box hitbox = attack_frames[i].frames[sword_get_frame(player)];
                if (hitbox.enabled)
                {
                    int offset = (player->is_right_facing) ? player->hurtbox.width : -hitbox.width;
                    sword->hitbox.top_left.x = hitbox.top_left.x * scale_factor + player->hurtbox.top_left.x + offset;  
                    sword->hitbox.top_left.y = hitbox.top_left.y + player->hurtbox.top_left.y;
                    sword->hitbox.width = hitbox.width;
                    sword->hitbox.height = hitbox.height;
                }
                sword->hitbox.enabled = hitbox.enabled;
                break;
            }
        }
    } 
    else if (player->vel.x == 0.0 && player->vel.y == 0.0 && !player->is_dead && !player->is_crouching)
    {
        int offset = 0;
        int penalty = 0;
        switch (player->stance)
        {
            case HIGH:
                offset = STANCE_HIGH_OFFSET;
                penalty = STANCE_ANGLE_PENALTY;
                break;
            case LOW:
                offset =  STANCE_LOW_OFFSET;
                penalty = STANCE_ANGLE_PENALTY;
                break;
            default:
                offset = STANCE_MIDDLE_OFFSET;
                break;
        }
        Sword *sword = &player->sword;
        sword->pos.x = player->pos.x + scale_factor * (sword->hitbox.width / 2.0 + penalty);
        sword->pos.y = player->pos.y + offset;
        sword->hitbox.top_left.y = sword->hitbox.top_left.y + offset;
        sword->hitbox.enabled = true;
        sword->hitbox.height = SWORD_WIDTH;
        sword->hitbox.width = SWORD_LENGTH - penalty;
    }
    else
    {
        player->sword.hitbox.enabled = false;
    }
}

int sword_get_frame( PlayerState player )
{
    int current_frame = 0;
    for (int i = 0; i < STANCE_NOS; i++)
    {
        if (attack_datas[i].stance == player->stance)
        {
            double time_remaining = player->sword.internal.attack_timer;
            while (current_frame < MAX_ATTACK_FRAMES && time_remaining > attack_datas[i].attack_frames[current_frame])
            {
                time_remaining -= attack_datas[i].attack_frames[current_frame];
                current_frame++;
            }
            return current_frame;
        }
    }
    return -1;
}

bool sword_check_attack_over(Sword *sword, Stance stance)
{
    double duration;
    double delay;
    switch(stance) {
        case LOW:
            duration = LOW_ATTACK_DURATION;
            delay = LOW_ATTACK_DELAY;
            break;
        case MIDDLE:
            duration = MID_ATTACK_DURATION;
            delay = MID_ATTACK_DELAY;
            break;
        case HIGH:
            duration = HIGH_ATTACK_DURATION;
            delay = HIGH_ATTACK_DELAY;
            break;
        default:
            fprintf(stderr, "Error: unknown stance \n");
    }

    // TODO() get stance so we know what kind of attakc doing and check against correct delay
    if (sword->internal.attack_timer >= duration) 
    {
        // Attack over
        sword->internal.attack_timer = 0;
        sword->internal.attack_delay = delay;
        sword->hitbox.enabled = false;
        return true;
    }     
    return false;
}

static void sword_update_hitbox( Sword *sword ) {
    // sword->pos is center
    // Remember bigger y means lower
    sword->hitbox.top_left.x = sword->pos.x - sword->hitbox.width / 2.0;
    sword->hitbox.top_left.y = sword->pos.y - sword->hitbox.height / 2.0;
}
