#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_scancode.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/wait.h>
#include "chip8.h"
#include "emulator.h"
#include "platform.h"

int main(int argc, char** argv)
{
    if(argc < 2){
        fprintf(stderr, "Usage: %s <path_to_rom>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    sdl_t sdl = {0};
    emu_context_t emu_ctx = {0};

    init_emu_context(&emu_ctx, argv[1]);
    init_sdl(&sdl, &emu_ctx);
    clear_renderer(&sdl, emu_ctx.bg_color);

    chip8_context_t chip8_ctx = {0};
    init_chip8(&emu_ctx, &chip8_ctx);

    //Event handler
    SDL_Event e;

    bool quit = false;
    bool step = false;
    //While application is running
    while( emu_ctx.state != QUIT)
    {
        //Handle events on queue
        process_input(&emu_ctx, &chip8_ctx );
        if(step){
            instr_t instr = fetch_instr(&chip8_ctx);
            debug_print_instr(&instr, &chip8_ctx); 
            execute_instr(&chip8_ctx, &instr);
        }
        step = false;
        for(int i = 0; i < emu_ctx.clock_speed/60;i++){
            instr_t instr = fetch_instr(&chip8_ctx);
            execute_instr(&chip8_ctx, &instr);
        }
        SDL_Delay(17); //TODO make this more accurate
        update_renderer(&sdl, &emu_ctx, &chip8_ctx);
        decrement_timers(&chip8_ctx); 
    }

    destroy_sdl(&sdl);

    return (0);
}
