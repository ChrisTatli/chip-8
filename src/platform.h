#ifndef PLATFORM_H
#define PLATFORM_H

#include "SDL2/SDL.h"
#include <stdbool.h>
#include "emulator.h"
#include "chip8.h"

typedef struct {    
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_AudioSpec obtained, desired;
} sdl_t;

void audio_callback(void*, uint8_t*, int);
bool init_sdl(sdl_t *, emu_context_t *);
void destroy_sdl(sdl_t *);
void clear_renderer(sdl_t *, color_t );
void update_renderer(sdl_t *, emu_context_t *, chip8_context_t *);
void process_input(emu_context_t* , chip8_context_t* );

#endif // DEBUG
