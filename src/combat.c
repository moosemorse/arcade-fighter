#include <stdio.h>
#include "player.h"
#include "combat.h"
#include "game_types.h"

static bool box_collision(Box box1, Box box2);

void combat_update(PlayerState player1, PlayerState player2) 
{
    // TODO() check for sword protection/collision using stances and current action
    if (box_collision(player1->hurtbox, player2->sword.hitbox))
    {
        printf("Player 1 died collision!\n");
        player_set_death_state(player1);
        return;
    } 
    else if (box_collision(player2->hurtbox, player1->sword.hitbox))
    {
        player_set_death_state(player2);
        printf("Player 2 died collision!\n");
        return;
    }
    else if (box_collision(player1->hurtbox, player2->hitbox))
    {
        player_receive_dive_kick(player1, player2);
        player_end_dive_kick(player2);
        printf("Player2 divekick/punch etc player 1");
    }
    else if (box_collision(player2->hurtbox, player1->hitbox))
    {
        player_receive_dive_kick(player2, player1);
        player_end_dive_kick(player1);
        printf("Player1 divekick/punch etc player 2");
    }
}

static bool box_collision( Box box1, Box box2 ) 
{
    double box1_xmin = box1.top_left.x;
    double box1_xmax = box1.top_left.x + box1.width;
    double box1_ymin = box1.top_left.y;
    double box1_ymax = box1.top_left.y + box1.height;

    double box2_xmin = box2.top_left.x;
    double box2_xmax = box2.top_left.x + box2.width;
    double box2_ymin = box2.top_left.y;
    double box2_ymax = box2.top_left.y + box2.height;

    bool x_axis_collides = box1_xmax > box2_xmin && box2_xmax > box1_xmin;
    bool y_axis_collides = box1_ymax > box2_ymin && box2_ymax > box1_ymin;
    
    return box1.enabled && box2.enabled && x_axis_collides && y_axis_collides;
}

