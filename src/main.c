#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/wait.h>
#include "chip8.h"
#include "emulator.h"

typedef struct {    
    SDL_Window *window;
    SDL_Renderer *renderer;

} sdl_t;

bool init_sdl(sdl_t *sdl, emu_context_t *emu){
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO);
    sdl->window = SDL_CreateWindow("chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED
                                   , emu->width * emu->pixel_scale 
                                   , emu->height * emu->pixel_scale
                                   , 0);
    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, 0);
    return true;
}

void destroy_sdl(sdl_t *sdl){
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}

void clear_renderer(sdl_t *sdl, color_t color){
    uint8_t r = (color >> 24) & 0xFF;
    uint8_t g = (color >> 16) & 0xFF;
    uint8_t b = (color >>  8) & 0xFF;
    uint8_t a = (color >>  0) & 0xFF;
    SDL_SetRenderDrawColor(sdl->renderer, r, g, b, a);
    SDL_RenderClear(sdl->renderer);
    SDL_RenderPresent(sdl->renderer);
}

void update_renderer(sdl_t *sdl, emu_context_t *emu, chip8_context_t *chip8){
    SDL_Rect rect = {.x = 0, .y = 0, .w = emu->pixel_scale, .h = emu->pixel_scale};
    
    for(uint32_t i = 0; i < emu->width*emu->height; i++){
        rect.x = (i % emu->width) * emu->pixel_scale;
        rect.y = (i / emu->width) * emu->pixel_scale;

        if(chip8->display_buffer[i]){
            color_t fg_color = emu->fg_color;
            uint8_t r = (fg_color >> 24) & 0xFF;
            uint8_t g = (fg_color >> 16) & 0xFF;
            uint8_t b = (fg_color >> 8) & 0xFF;
            uint8_t a = (fg_color >> 0) & 0xFF;
            SDL_SetRenderDrawColor(sdl->renderer, r, g, b, a);
            SDL_RenderFillRect(sdl->renderer, &rect);
        } else {
            color_t bg_color = emu->bg_color;
            uint8_t r = (bg_color >> 24) & 0xFF;
            uint8_t g = (bg_color >> 16) & 0xFF;
            uint8_t b = (bg_color >> 8) & 0xFF;
            uint8_t a = (bg_color >> 0) & 0xFF;
            SDL_SetRenderDrawColor(sdl->renderer, r, g, b, a);
            SDL_RenderFillRect(sdl->renderer, &rect);
        }
    }
    SDL_RenderPresent(sdl->renderer);
}

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
    while( !quit )
    {
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 ) // poll for event
        {
            //User requests quit
            if( e.type == SDL_QUIT ) // unless player manually quits
            {
                quit = true;
            }
            if(e.type == SDL_KEYDOWN){
                switch (e.key.keysym.sym) {
                    case SDLK_SPACE:
                        step = true;
                        break;

                }
            }
        }
        if(step){
            instr_t instr = fetch_instr(&chip8_ctx);
            debug_print_instr(&instr, &chip8_ctx); 
            execute_instr(&chip8_ctx, &instr);
        }
        step = false;
        /**for(int i = 0; i < emu_ctx.clock_speed/60;i++){
            instr_t instr = fetch_instr(&chip8_ctx);
            execute_instr(&chip8_ctx, &instr);
        }**/
        SDL_Delay(17);
        update_renderer(&sdl, &emu_ctx, &chip8_ctx);

    }
    
    destroy_sdl(&sdl);

    return (0);
}
