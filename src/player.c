#include <stdio.h>
#include <stdlib.h>
#include "player.h"
#include "game_types.h"
#include "input.h"
#include "sword.h"

//TODO() Hassam: Temp remove this later - just here for compilation purpose
// instead we should be passing in the spawn position for our players in player_init 
// or pass in the screen size to player_init
#define SCREEN_SIZE_X 1280 
#define SCREEN_SIZE_Y 720 

/* Main player constants */  
#define JUMP_SPEED 35.0
#define PLAYER_WIDTH 50.0 
#define PLAYER_HEIGHT 150.0 
#define PLAYER_SPEED 450.0
#define CROUCHED_HEIGHT (PLAYER_HEIGHT / 2.0)

/* Dive kick constants */
#define DIVE_KICK_HITBOX_WIDTH 40.0
#define DIVE_KICK_HITBOX_HEIGHT 30.0
#define DIVE_KICK_HITBOX_XOFFSET 40.0
#define DIVE_KICK_HITBOX_YOFFSET -20.0

#define DIVE_KICK_HORIZONTAL_VEL 700.0
#define DIVE_KICK_VERTICAL_VEL 600.0
#define DIVE_KICK_STUN_TIME 1.2
#define DIVE_KICK_X_IMPACT 20.0
#define DIVE_KICK_Y_IMPACT (JUMP_SPEED * 0.75)
#define DIVE_KICK_RECOVERY_TIME 0.6

/* Game physics constants */
#define GRAVITY 90.0   
#define GROUND_LEVEL (SCREEN_SIZE_Y)

/* Internal player constants */
#define JUMP_DELAY 0.1
#define STANCE_DELAY 0.2

static void player_update_state( PlayerState player, PlayerInput input, double dt );
static void set_jump_state( PlayerState player, PlayerInput input, double dt );
static void set_attack_state( PlayerState player, PlayerInput input, double dt );
static void set_hurtbox_size( PlayerState player );
static void player_simulate_physics( PlayerState player, double dt );
static void player_begin_dive_kick( PlayerState player );

static bool handle_ground_collision( PlayerState player );
static void set_player_hurtbox_to_ground( PlayerState player );
static void handle_wall_collision( PlayerState player );

static void set_player_hitbox( PlayerState player );
static void set_player_hurtbox( PlayerState player );
static void update_stance( PlayerState player, JoystickPos movement ); 
static void adjust_orientation( PlayerState player );

static void set_stunned(PlayerState player, double stun_time);

static void internals_init( PlayerState player );
static void internals_update( PlayerState player, double dt ); 

// Must free the internals
void player_init( PlayerState player, PlayerId id ) 
{
    player->id = id; 
    player->time_in_anim = 0;
    player->pos.x = id == PLAYER_1 ? PLAYER_WIDTH / 2.0 : SCREEN_SIZE_X - PLAYER_WIDTH / 2.0;
    player->pos.y = GROUND_LEVEL - PLAYER_HEIGHT / 2.0;

    player->vel.x = 0.0;
    player->vel.y = 0.0;
    player->stance = MIDDLE;
    // player->is_armed = true;
    player->is_jumping = false;
    player->is_attacking = false;
    player->is_dead = false;
    player->is_right_facing = id == PLAYER_1;
    player->in_stunned_state = false;
    player->is_crouching = false;
    
    player->hitbox.width = 0.0;
    player->hitbox.height = 0.0;
    set_player_hitbox(player);
    player->hitbox.enabled = false;

    player->hurtbox.width = PLAYER_WIDTH;
    player->hurtbox.height = PLAYER_HEIGHT;
    set_player_hurtbox(player);
    player->hurtbox.enabled = true;
    
    player->sword = sword_init(player);
    
    internals_init(player);
}

static void internals_init( PlayerState player ) {
    player->internal.jump_delay = 0.0;
    player->internal.stance_delay = 0.0;
    player->internal.is_stunned = false;
    player->internal.stunned_duration = 0.0;
}

void player_set_death_state( PlayerState player )
{
    player->is_dead = true;
    //Purposefully don't set y velocity and is_jumping so if killed in mid air dies and falls
    player->vel.x = 0;
    player->is_attacking = false;

    player->hurtbox.enabled = false;
    player->sword.hitbox.enabled = false;
}

void player_update( PlayerState player, PlayerInput input, double dt ) 
{
    internals_update(player, dt);

    if( !player->is_dead && !player->internal.is_stunned )
    {
        player_update_state(player, input, dt);
    }

    set_hurtbox_size(player);

    player_simulate_physics(player, dt);

    // TODO() If sword not thrown and check armed aswell 
    if (!player->internal.is_stunned) {
        move_sword_to_player(&player->sword, player->pos, player->is_right_facing, player->stance);
        sword_update_attack_hitbox( player );
    }
}

static void player_update_state( PlayerState player, PlayerInput input, double dt ) 
{
    // Sets initial jumping velocity - checks if allowed to jump etc 
    // simulate_physics handles the jumping effect
    
    set_jump_state(player, input, dt);
    
    set_attack_state(player, input, dt);

    // TODO() depends if running or not will have field in player for that
    if (player->is_attacking && player->is_jumping)
    {
        //Dive kick - x and y velocity unchanged
    }
    else if (player->is_attacking) 
    {
        //Sword attack / melee punch
        player->vel.x = 0.0;
    }
    else
    {
        // Not attacking
        if (!player->is_jumping)
        {
            update_stance(player, input.joystick_pos);
        }
        // If crouching we set velocity here just for changing orientation
        player->vel.x = input.move_x * PLAYER_SPEED * dt;
    }
    
    // Adjusts according to velocity
    adjust_orientation(player); 
    if (player->is_crouching)
    {
        player->vel.x = 0.0;
    }

}

static void set_hurtbox_size( PlayerState player )
{
    double previous_lowest = player->hurtbox.top_left.y + player->hurtbox.height;
    //TODO() upate pos accordingly
    if (player->is_crouching)
    {
        // Crouching can only occur from idle state
        player->hurtbox.width = PLAYER_WIDTH;
        player->hurtbox.height = CROUCHED_HEIGHT;
    }
    //TODO() have case for stunned - shouldnt set player_hurtbox_to_ground tho
    else
    {
        // In General if hitbox growing - likely for hitbox to phase into ground 
        // dont necessarily want to always force to ground - so call handle_ground_collision
        player->hurtbox.width = PLAYER_WIDTH;
        player->hurtbox.height = PLAYER_HEIGHT;
    }
    player->pos.y = previous_lowest - player->hurtbox.height / 2.0;
    // simulate_physics will set the player hurtbox location to snap on to new pos aswell
}

static void set_jump_state( PlayerState player, PlayerInput input, double dt ) 
{
    bool already_jumping = player->is_jumping;
    
    if( !already_jumping && player->internal.jump_delay <= 0.0 && !player->is_attacking  && !player->is_crouching ) 
    {
        player->is_jumping = input.jump_pressed;
        // If started jumping basically
        // TODO() do we need to check grounded?
        if (player->is_jumping) 
        {
            player->vel.y = -JUMP_SPEED;
        }
    }
}

static void set_attack_state( PlayerState player, PlayerInput input, double dt )
{
    sword_update_internal_state(&player->sword, player->is_attacking, dt);
    
    //Beginning sword attack
    if (!player->is_attacking && sword_can_attack(&player->sword) && !player->is_jumping && !player->is_crouching)
    {
        player->is_attacking = input.attack_pressed;
        if (player->is_attacking)
        {
            sword_begin_melee_attack(&player->sword, player->stance);
        }
    }
    // Dive kick
    else if (!player->is_attacking && player->is_jumping && input.attack_pressed)
    {
        player_begin_dive_kick(player);
    }
    // Check attacking but not dive_kick
    else if (player->is_attacking && !player->is_jumping) {
        player->is_attacking = !sword_check_attack_over(&player->sword, player->stance);
    }
}

static void player_simulate_physics( PlayerState player, double dt ) 
{
    // Normal jumps and movement should have gravity so dt taken into account with velocity
    // Whereas when dive kicking fixed fall speed and horizontal speed so dt must be taken into account when updating position
    if( !(player->is_attacking && player->is_jumping) )
    {
        player->pos.x += player->vel.x;
        player->pos.y += player->vel.y;
    }
    else {
    // Dive kicking
        player->pos.x += player->vel.x * dt;
        player->pos.y += player->vel.y * dt;
    }
    
    // Move hitbox according to new pos
    set_player_hurtbox(player); 

    handle_wall_collision(player);
   
    if( player->is_jumping) {
        if( !player->is_attacking ) 
        {
            // Downward acceleration due to gravity - if not dive kicking
            player->vel.y += GRAVITY * dt;
        }
        
        bool clamped = handle_ground_collision(player);
        if (clamped) 
        {
            player->is_jumping = false;
            player->internal.jump_delay = JUMP_DELAY;
            
            // In case dive kicking
            player->is_attacking = false;
            player->hitbox.enabled = false;

            // If is stunned and land on floor - dont move
            if (player->internal.is_stunned)
            {
                player->vel.x = 0.0;
            }
        }
    }
    
    set_player_hurtbox(player);
    set_player_hitbox(player);
}

 // Update player hurtbox based on current (central) pos
static void set_player_hurtbox( PlayerState player ) 
{ 
    player->hurtbox.top_left = (Vector_2D){player->pos.x - player->hurtbox.width / 2.0, player->pos.y - player->hurtbox.height / 2.0};
    // Width and height of hurtbox unchanged here
}


static void set_player_hitbox( PlayerState player ) {
    if( player->is_attacking && player->is_jumping )
    // Divekick
    {
        double scale_factor = (player->is_right_facing) ? 1.0 : -1.0;
        double center_x = player->hurtbox.top_left.x + player->hurtbox.width / 2.0;        

        player->hitbox.top_left.x = center_x - player->hitbox.width / 2.0 + scale_factor * DIVE_KICK_HITBOX_XOFFSET;

        player->hitbox.top_left.y = player->hurtbox.top_left.y + player->hurtbox.height + DIVE_KICK_HITBOX_YOFFSET;
    }
}


static void update_stance( PlayerState player, JoystickPos stance_change )
{
    //bool previously_crouching = player->is_crouching;
    //player->is_crouching = false;

    if (player->internal.stance_delay <= 0.0)
    {
        player->is_crouching = false;
        if( stance_change == JOYSTICK_UP ) 
        {
            // TODO() if high already and holding joystick up - get ready to throw sword?
            player->stance = (player->stance == LOW) ? MIDDLE : HIGH;
            player->internal.stance_delay = STANCE_DELAY;
        } 
        else if( stance_change == JOYSTICK_DOWN )
        {
            // TODO() perhaps if already lowest and holding low we crouch?
            if (player->stance == LOW) {
                player->is_crouching = true;
            }   
            player->stance = (player->stance == HIGH) ? MIDDLE : LOW;
            player->internal.stance_delay = STANCE_DELAY;
        }
    }
}

static void player_begin_dive_kick( PlayerState player )
{
    double scale_factor = (player->is_right_facing) ? 1.0 : -1.0;

    player->is_attacking = true;
    player->vel.x = DIVE_KICK_HORIZONTAL_VEL * scale_factor;
    player->vel.y = DIVE_KICK_VERTICAL_VEL;
    
    player->hitbox.enabled = true;
    player->hitbox.width = DIVE_KICK_HITBOX_WIDTH;
    player->hitbox.height = DIVE_KICK_HITBOX_HEIGHT;
    // Hitbox top_left set later
}

void player_end_dive_kick( PlayerState player )
{
    double scale_factor = (player->is_right_facing) ? 1.0 : -1.0;

    player->is_attacking = false;
    // TODO() remove magig numbers
    player->vel.x = -scale_factor * DIVE_KICK_X_IMPACT * 0.25;
    //Gravity should make them fall
    player->vel.y = -DIVE_KICK_Y_IMPACT * 0.5;

    player->internal.is_stunned = true;
    player->internal.stunned_duration = DIVE_KICK_RECOVERY_TIME;

    player->hitbox.enabled = false;

}

void player_receive_dive_kick( PlayerState receiver, PlayerState attacker )
{
    set_stunned(receiver, DIVE_KICK_STUN_TIME);
   
    double scale_factor = (attacker->is_right_facing) ? 1.0 : -1.0;

    receiver->vel.x = DIVE_KICK_X_IMPACT * scale_factor;
    receiver->vel.y = -DIVE_KICK_Y_IMPACT;
    
    //TODO() probably not needed but should not matter
    set_hurtbox_size(receiver);
}

static void set_stunned( PlayerState player, double stun_time )
{
    //TODO() change hitbox to dramatically shrink

    player->in_stunned_state = true;
    player->internal.is_stunned = true;
    player->internal.stunned_duration = stun_time;
    player->is_jumping = true;
    player->is_attacking = false;
    player->hitbox.enabled = false;
    player->sword.hitbox.enabled = false;
}

static void set_unstunned( PlayerState player )
{
    //TODO() grow hitbox as necessary
    player->in_stunned_state = false;
    player->internal.is_stunned = false;
}

static void adjust_orientation( PlayerState player ) 
{
    if (player->vel.x > 0 && !player->is_right_facing) 
    {
        player->is_right_facing = true;
    } 
    else if (player->vel.x < 0 && player->is_right_facing) 
    {
        player->is_right_facing = false;
    }
}

// Returns true if clamped position - false if no need
static bool handle_ground_collision( PlayerState player ) 
{   
    // Checking if bottom left of player touching ground (note bottom will have highest y value)
    if( player->hurtbox.top_left.y + player->hurtbox.height >= GROUND_LEVEL && player->vel.y > 0 )
    {
        player->vel.y = 0;
        set_player_hurtbox_to_ground(player);
        return true;
    }
    return false;
}

static void set_player_hurtbox_to_ground( PlayerState player )
{
    player->pos.y = GROUND_LEVEL - player->hurtbox.height / 2.0;
}

static void handle_wall_collision( PlayerState player ) 
{
    //TODO() make players bounce of wall -> have player_stasis which is bool if true -> user does not control user velocity

    if( player->hurtbox.top_left.x < 0 )
    {
        player->vel.x = 0;
        player->pos.x = player->hurtbox.width / 2.0;
    }
    else if( player->hurtbox.top_left.x + player->hurtbox.width > SCREEN_SIZE_X )
    {
        player->vel.x = 0;
        player->pos.x = SCREEN_SIZE_X - player->hurtbox.width / 2.0;
    }
}

static void update_delay( double *delay, double dt )
{
    if( *delay > 0.0 ) 
    {
        *delay -= dt;
        if( *delay < 0.0 )
        {
            *delay = 0.0;
        }
    }
}

static void internals_update( PlayerState player, double dt )
{
    // Updating Jump delay - to see if we are allowed to jump
    update_delay(&player->internal.jump_delay, dt);
    update_delay(&player->internal.stance_delay, dt);
    update_delay(&player->internal.stunned_duration, dt);

    if (player->internal.stunned_duration <= 0.0)
    {
        set_unstunned(player);
    }
}

void print_vector_2d( Vector_2D v ) 
{
    printf("(x: %.2f, y: %.2f)", v.x, v.y);
}


//TODO() refactor:
void print_player( PlayerState player ) 
{
    printf("=== Player State ===\n");
    printf("ID: %d\n", player->id);
    printf("Time_in_anim #: %f\n", player->time_in_anim );
    printf("Position: ");
    print_vector_2d(player->pos);
    printf("\nVelocity: ");
    print_vector_2d(player->vel);
    printf("\nStance: ");
    switch (player->stance) 
    {
        case LOW: printf("LOW\n"); break;
        case MIDDLE: printf("MIDDLE\n"); break;
        case HIGH: printf("HIGH\n"); break;
        default: printf("UNKNOWN\n"); break;
    }
    printf("Is Jumping: %s\n", player->is_jumping ? "Yes" : "No");
    printf("Is Attacking: %s\n", player->is_attacking ? "Yes" : "No");
    printf("Facing Right: %s\n", player->is_right_facing ? "Yes" : "No");

    printf("Hurtbox Top Left: ");
    print_vector_2d(player->hurtbox.top_left);
    printf("\nHurtbox Size: %.2f x %.2f\n", player->hurtbox.width, player->hurtbox.height);

    printf("Sword Center: ");
    print_vector_2d(player->sword.pos);
    printf("\nSword Hitbox Top Left: ");
    print_vector_2d(player->sword.hitbox.top_left);
    printf("\nSword Size: %.2f x %.2f\n", player->sword.hitbox.width, player->sword.hitbox.height);
    printf("====================\n");
}
