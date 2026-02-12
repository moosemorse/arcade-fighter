#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include "renderer.h"
#include "game_types.h"
#include "sword.h"

#define BACKGROUND_FRAMES 11
#define TIME_PER_BACKGROUND 0.1
#define DEFAULT_TIME_PER_FRAME 0.1
#define DEATH_TIME_PER_FRAME 0.4
#define NO_ROWS 20
#define MAX_COLS 9
#define CROUCH_ROW 0
#define KICK_ROW 1
#define KICK_DISARMED_ROW 2
#define HIGH_ATTACK_ROW 3
#define MIDDLE_ATTACK_ROW 4
#define LOW_ATTACK_ROW 5
#define DEATH_ROW 6
#define DEATH_DISARMED_ROW 7
#define FALL_ROW 8
#define FALL_DISARMED_ROW 9
#define IDLE_LOW_ROW 10
#define IDLE_DISARMED_ROW 11
#define IDLE_HIGH_ROW 12
#define IDLE_MIDDLE_ROW 13
#define JUMP_ROW 14
#define JUMP_DISARMED_ROW 15
#define RUN_ROW 16
#define RUN_DISARMED_ROW 17
#define STUNNED_ROW 18
#define THROW_ROW 19
#define FRAMES_CROUCH 1
#define FRAMES_STUN 6
#define FRAMES_THROW 5
#define FRAMES_ATTACK 4
#define FRAMES_DEATH 9
#define FRAMES_JUMP 2
#define FRAMES_IDLE 6
#define FRAMES_FALL 2
#define FRAMES_RUN 8
#define FRAMES_KICK 2
#define SPRITE_WIDTH_SCALE 8
#define SPRITE_HEIGHT_SCALE 2
#define SPRITE_TOP_PADDING 0
#define SPRITE_SIDE_PADDING 0
#define RIGHT_OFFSET 10
#define LEFT_OFFSET -10
#define TOP_OFFSET 30

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} Game;

Game game = {
    .window = NULL,
    .renderer = NULL,
};

typedef struct {
    SDL_Rect rect;
    SDL_Texture *spritesheet_image;
} Spritesheet;

static Spritesheet player_sprite;
static Spritesheet background;
static double background_state = 0;

static void renderer_draw_player_hitbox( PlayerState player );
static void renderer_draw_sword( PlayerState player );
static Spritesheet create_spritesheet( char const *path, int rows, int columns );
static void draw_sprite( Spritesheet spritesheet, SDL_Rect *position, int row, int column, bool flip );
static int state_select( PlayerState player );
static void update_player_frame( PlayerState player, double dt );

static double PLAYER_NORMAL_HEIGHT;
static double PLAYER_NORMAL_WIDTH;

void renderer_init( void ) 
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0)
    {
        fprintf(stderr, "Could not initalise: Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    game.window = SDL_CreateWindow("Extension group project", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_SIZE_X, SCREEN_SIZE_Y, SDL_WINDOW_SHOWN);
    assert(game.window != NULL);
    
    game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderClear(game.renderer);
    
    IMG_InitFlags imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init( imgFlags ) & imgFlags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
    }

    player_sprite = create_spritesheet("../assets/spritesheet.png", NO_ROWS, MAX_COLS);

    background = create_spritesheet("../assets/background.png", 1, BACKGROUND_FRAMES);
    
    SDL_UpdateWindowSurface(game.window);
}

void renderer_set_player_size( double height, double width )
{
    PLAYER_NORMAL_HEIGHT = height;
    PLAYER_NORMAL_WIDTH = width;
}

void renderer_begin_frame( void )
{
    SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
    SDL_RenderClear(game.renderer);
}

void renderer_draw_player( PlayerState player, double dt )
{
    SDL_Rect hurtbox_rect = {
        player->hurtbox.top_left.x,
        player->hurtbox.top_left.y,
        player->hurtbox.width,
        player->hurtbox.height
    };
    SDL_SetRenderDrawColor(game.renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(game.renderer, &hurtbox_rect);
    
    renderer_draw_player_hitbox(player);   

    update_player_frame(player, dt);
    SDL_Rect sprite_rect = {   
            player->pos.x, 
            player->pos.y, 
            PLAYER_NORMAL_WIDTH,
            PLAYER_NORMAL_HEIGHT
        };

    // Only non-kick attacks have special animations
    int current_frame = (player->is_attacking && !player->is_jumping) ? sword_get_frame(player) : player->time_in_anim / ((player->is_dead) ? DEATH_TIME_PER_FRAME : DEFAULT_TIME_PER_FRAME);
    draw_sprite(player_sprite, &sprite_rect, state_select(player), current_frame , !player->is_right_facing);
    
    //TODO() we check here and in function??
    if( player->sword.hitbox.enabled ) {
        renderer_draw_sword(player);
    }
}

/*
 * Usage: quit = SDL_event(handler)
 * Waits until sdl events occur, returns true when needing to quit
*/
bool SDL_event_handler( void )
{
    bool quit = false;
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0) {
        if(e.type == SDL_QUIT) {
            quit = true;
        }
    }
    return quit;
}

static void update_player_frame( PlayerState player, double dt )
{
    int max_frames = 0;
    bool no_loop = false;
    double time_per_frame = DEFAULT_TIME_PER_FRAME;
    switch (state_select(player))
    {
        case JUMP_ROW:
        case JUMP_DISARMED_ROW:
            max_frames = FRAMES_JUMP;
            break;
        case FALL_ROW:
        case FALL_DISARMED_ROW:
            max_frames = FRAMES_FALL;
            break;
        case RUN_ROW:
        case RUN_DISARMED_ROW:
            max_frames = FRAMES_RUN;
            break;
        case IDLE_MIDDLE_ROW:
        case IDLE_HIGH_ROW:
        case IDLE_LOW_ROW:
        case IDLE_DISARMED_ROW:
            max_frames = FRAMES_IDLE;
            break;
        case HIGH_ATTACK_ROW:
        case MIDDLE_ATTACK_ROW:
        case LOW_ATTACK_ROW:
            max_frames = FRAMES_ATTACK;
            no_loop = true;
            break;
        case DEATH_ROW:
        case DEATH_DISARMED_ROW:
            max_frames = FRAMES_DEATH;
            no_loop = true;
            time_per_frame = DEATH_TIME_PER_FRAME;
            break;
        case CROUCH_ROW:
            max_frames = FRAMES_CROUCH;
            break;
        case STUNNED_ROW:
            max_frames = FRAMES_STUN;
            break;
        case THROW_ROW:
            max_frames = FRAMES_THROW;
            break;
        case KICK_ROW:
        case KICK_DISARMED_ROW:
            printf("In kick state, frame %f\n", player->time_in_anim / time_per_frame);
            max_frames = FRAMES_KICK;
            break;
        default:
            fprintf(stderr, "Player is in invalid state!!\n");
    }
    assert(max_frames != 0);
    player->time_in_anim += dt;
    if ((player->time_in_anim / time_per_frame) >= max_frames)
    {
        if (no_loop)
        {
            player->time_in_anim -= dt; 
        } else
        {
            player->time_in_anim = 0;
        }
    }
}

static int state_select( PlayerState player )
{
    if (player->is_dead)
    {
        return DEATH_ROW;
    }
    if (player->is_jumping)
    {
        if (player->is_attacking)
        {
            return KICK_ROW;
        }
        else if (player->vel.y < 0.0)
        {
            return JUMP_ROW;
        }
        else
        {
            return FALL_ROW;
        }
    }
    if (player->in_stunned_state)
    {
        return STUNNED_ROW;
    }
    if (player->is_crouching)
    {
        return CROUCH_ROW;
    }
    if (player->vel.x != 0.0)
    {
        return RUN_ROW;
    }
    if (player->is_attacking)
    {
        switch (player->stance)
        {
            case HIGH:
                return HIGH_ATTACK_ROW;
            case MIDDLE:
                return MIDDLE_ATTACK_ROW;
            case LOW:
                return LOW_ATTACK_ROW;
        }
    }
    switch (player->stance)
    {
        case HIGH:
            return IDLE_HIGH_ROW;
        case MIDDLE:
            return IDLE_MIDDLE_ROW;
        case LOW:
            return IDLE_LOW_ROW;
        default:
            fprintf(stderr, "Invalid stance");
            return 0;
    }
}

static void renderer_draw_player_hitbox( PlayerState player )
{
    if (player->hitbox.enabled)
    {
        SDL_Rect rect = { player->hitbox.top_left.x, 
        player->hitbox.top_left.y,
        player->hitbox.width, player->hitbox.height
        };
        //printf("Draw: x:%i, y: %i, w: %i, h, %i\n", rect.x, rect.y, rect.w, rect.h);
        SDL_SetRenderDrawColor(game.renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(game.renderer, &rect);
    }
}


static void renderer_draw_sword( PlayerState player )
{
    if (player->sword.hitbox.enabled)
    {
        SDL_Rect rect = { player->sword.hitbox.top_left.x, 
        player->sword.hitbox.top_left.y,
        player->sword.hitbox.width, player->sword.hitbox.height
        };
        //printf("Draw: x:%i, y: %i, w: %i, h, %i\n", rect.x, rect.y, rect.w, rect.h);
        SDL_SetRenderDrawColor(game.renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(game.renderer, &rect);
    }
}

void renderer_end_frame( void )
{
    SDL_RenderPresent(game.renderer);
}

void renderer_clean( void )
{
    if (player_sprite.spritesheet_image) {
        SDL_DestroyTexture(player_sprite.spritesheet_image);
        player_sprite.spritesheet_image = NULL;
    }

    if (background.spritesheet_image) {
        SDL_DestroyTexture(background.spritesheet_image);
        background.spritesheet_image = NULL;
    }

    if (game.renderer) {
        SDL_DestroyRenderer(game.renderer);
        game.renderer = NULL;
    }

    if (game.window) {
        SDL_DestroyWindow(game.window);
        game.window = NULL;
    }
    SDL_GL_UnloadLibrary();

    IMG_Quit();
    SDL_Quit();
}

static Spritesheet create_spritesheet( char const *path, int rows, int columns )
{
    SDL_Surface *surface = IMG_Load( path );
    if (!surface)
    {
        fprintf(stderr, "Failed surface load\n");
        exit(EXIT_FAILURE);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(game.renderer, surface);
    if (!texture)
    {
        fprintf(stderr, "Failed texture load\n");
        exit(EXIT_FAILURE);
    }

    Spritesheet new = {
        .spritesheet_image = texture,
        .rect = {
            .w = surface->w / columns,
            .h = surface->h / rows,
        }
    };
    SDL_FreeSurface(surface);

    return new;
}

/*
 * Usage: draw_sprite(spritesheet, position, player_state_row, frame, flip)
 * Draws sprite at given rect (center) position and size, at given frame number, with
 * boolean at end telling whether to flip sprite
*/
static void draw_sprite( Spritesheet spritesheet, SDL_Rect *position, int row, int column, bool flip )
{
    SDL_Rect sheet_rect = {
        .w = spritesheet.rect.w - 2 * SPRITE_SIDE_PADDING,
        .h = spritesheet.rect.h - 2 * SPRITE_TOP_PADDING,
        .x = column * spritesheet.rect.w + SPRITE_SIDE_PADDING,
        .y = row * spritesheet.rect.h + SPRITE_TOP_PADDING
    };
    
    SDL_Rect draw_rect = {
        .w = position->w * SPRITE_WIDTH_SCALE,
        .h = position->h * SPRITE_HEIGHT_SCALE,
        .x = position->x - (position->w * SPRITE_WIDTH_SCALE / 2) + (flip ? LEFT_OFFSET : RIGHT_OFFSET),
        .y = position->y - (position->h * (SPRITE_HEIGHT_SCALE - 1.0 / 2.0) - TOP_OFFSET ) 
    };

    SDL_RenderCopyEx(game.renderer, spritesheet.spritesheet_image, &sheet_rect, &draw_rect, 0, NULL, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

static void render_background( Spritesheet spritesheet, int frame )
{
    spritesheet.rect.x = spritesheet.rect.w * frame;
    spritesheet.rect.y = 0; // Frames are all horizontal

    SDL_RenderCopy(game.renderer, spritesheet.spritesheet_image, &spritesheet.rect, NULL);
}

void renderer_draw_background( double dt )
{
    background_state += dt;
    if (background_state / TIME_PER_BACKGROUND >= BACKGROUND_FRAMES)
    {
        background_state = 0;
    }
    render_background( background, (background_state / TIME_PER_BACKGROUND) );
}
