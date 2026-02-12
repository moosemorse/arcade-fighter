#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "player.h"
#include "input.h"
#include "renderer.h"
#include "combat.h"
#include "game_types.h"
#include "timer.h"
#include "keyboard.h"

#define SCREEN_FPS 60
#define SCREEN_TICKS_PER_FRAME (1000.0 / SCREEN_FPS)  //1 second

#define DEATH_TIME 5.0

bool using_keyboard = true;

int main( void ) {
    renderer_init();
    
    // Using calloc in case forget to initialise everything
    PlayerState player1 = calloc(1, sizeof(struct PlayerState));
    assert(player1 != NULL);
    PlayerState player2 = calloc(1, sizeof(struct PlayerState));
    assert(player2 != NULL);
    //TODO() hassam: should set up our PlayerState structs correctly
    // Right now using playerstate not as a pointer to struct but just struct
    player_init(player1, PLAYER_1); player_init(player2, PLAYER_2);
    
    // Set Player default height and width internally in renderer so sprites draw correctly
    renderer_set_player_size(player1->hurtbox.height, player1->hurtbox.width);

    PlayerInput p1_input, p2_input;
    
    // Getting FPS - measure how long the frame takes to run 
    double fps;
    fps=0;
    fps=fps; // unused variable  warning remove TODO

    Timer fps_timer = timer_create();
    int counted_frames = 0;
    timer_start(fps_timer);
    
    // Used for capping FPS
    Timer cap_timer = timer_create();

    // Measuring dt
    double dt; // Measured in second
    uint64_t current_frame_counter;
    uint64_t prev_frame_counter = SDL_GetPerformanceCounter();
    uint64_t ticks_per_second = SDL_GetPerformanceFrequency();
    
    Timer death_timer = timer_create();
    bool player_died = false;
    bool quit = false;
    
    // window open
    while( !quit ) {
        /* ------- GAME LOOP SETUP -------*/

        //Start/restart Cap timer - to see how long frame takes to run
        timer_start(cap_timer);

        //TODO() lawrence: Event handler function is here
        quit = SDL_event_handler();
        
        // Calculating Delta Time
        current_frame_counter = SDL_GetPerformanceCounter();
        dt = (double) (current_frame_counter - prev_frame_counter) / (double) ticks_per_second;
        prev_frame_counter = current_frame_counter;
        
        // Calculating - the FPS according to our fps timer
        double seconds_passed = timer_get_seconds(fps_timer);
        
        // First frame can sometimes have a very high fps do need to correct it
        fps = (seconds_passed < 0.15) ? 1 : (double) counted_frames / seconds_passed;
        
        //printf("fps = %lf   deltatime = %lf\n", fps, dt);


        /* ------- GAME LOOP UPDATES ------- */

        // input_get returns a PlayerInput struct for the corresponding player
        if (!using_keyboard) {
            p1_input = input_get(player1->id);
            p2_input = input_get(player2->id);
        } else {
            p1_input = (PlayerInput) {0.0, JOYSTICK_MID, false, false};
            p2_input = (PlayerInput) {0.0, JOYSTICK_MID, false, false};
            const uint8_t *keyboard_input = SDL_GetKeyboardState(NULL);

            set_player1_keyboard_input(&p1_input, keyboard_input);
            set_player2_keyboard_input(&p2_input, keyboard_input);
        }
        
        player_update(player1, p1_input, dt);
        player_update(player2, p2_input, dt);

        combat_update(player1, player2);

        if( !player_died && (player1->is_dead || player2->is_dead) )
        {
            timer_start(death_timer);
            player_died = true;
        }
        if( timer_get_seconds(death_timer) >= DEATH_TIME )
        {
            quit = true;
            printf("game over");
        }
        

        /* -------- GAME LOOP RENDERING ------- */

        //TODO() lawrence: should also draw/render background
        // You wil probably have some internal frame for the background (if animated)
        renderer_begin_frame(); 
        renderer_draw_background(dt);
        renderer_draw_player(player1, dt);
        renderer_draw_player(player2, dt);
        renderer_end_frame();


        /* ------- GAME LOOP Frame Capping ------- */

        counted_frames++;

        // Now we find how long our frame took - if too short then add a delay
        double frame_ticks = timer_get_seconds(cap_timer) * 1000;
        if( frame_ticks < SCREEN_TICKS_PER_FRAME ) {
            // SDL_Delay only takes in a uint32_t
            SDL_Delay((uint32_t) (SCREEN_TICKS_PER_FRAME - frame_ticks));
        }
    }

    // Close/free anything here
    free(player1);
    free(player2);
    timer_free(fps_timer);
    timer_free(cap_timer);
    timer_free(death_timer);
    renderer_clean();

    return 0;
    
}
