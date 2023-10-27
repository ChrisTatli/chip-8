
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
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
} emu_context_t;

bool init_sdl(sdl_t *sdl, emu_context_t *emu){
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO);
    SDL_CreateWindowAndRenderer(256,256 , 0, &sdl->window, &sdl->renderer);
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

int main(void)
{
    sdl_t sdl = {};
    emu_context_t emu_ctx = { .bg_color = 0xFF000000};

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
