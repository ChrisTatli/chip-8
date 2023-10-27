#include <SDL2/SDL_log.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>





typedef uint32_t color_t;

typedef struct {    
    SDL_Window *window;
    SDL_Renderer *renderer;

} sdl_t;

typedef struct{
    uint32_t height;
    uint32_t width;
    color_t bg_color;
    color_t fg_color;
    uint32_t pixel_scale;
    uint32_t clock_speed;
    FILE *rom;
    uint32_t rom_size;
} emu_context_t;

bool load_rom(char *, emu_context_t* );


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

void init_emu_context(emu_context_t *emu ){
    *emu =(emu_context_t){
        .width = 64,
        .height = 32,
        .bg_color = 0xFF000000,
        .fg_color = 0x00000000,
        .clock_speed = 700, //standard speed fits most chip8 roms
        .pixel_scale = 20
    };

    if(!load_rom("/Users/christophertatli/dev/chip-8/roms/IBMLogo.ch8", emu)){
        SDL_Log("Failed loading rom\n");
    }
}

bool load_rom(char *rom_path, emu_context_t* emu){
    emu->rom = fopen(rom_path, "rb");
    if(!emu->rom){
        SDL_Log("Failed loading %s, file is invalid or does not exist.\n", rom_path);
        return false;
    }
    
    fseek(emu->rom, 0, SEEK_END);
    emu->rom_size = ftell(emu->rom);
    rewind(emu->rom);

    uint32_t max_rom_size = (uint32_t)(4096 - 0x200); //TODO make this better

    if(emu->rom_size > max_rom_size){
        SDL_Log("Failed loading %s, ROM size exceeds %d bytes.\n", rom_path, max_rom_size);
        return false;
    }

    return true;

}

int main(void)
{
    sdl_t sdl = {};
    emu_context_t emu_ctx = {};
    init_emu_context(&emu_ctx);
    init_sdl(&sdl, &emu_ctx);
    clear_renderer(&sdl, emu_ctx.bg_color);
    //Event handler
    bool quit = false;
    SDL_Event e;

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
        }
    }
    
    destroy_sdl(&sdl);

    return (0);
}
