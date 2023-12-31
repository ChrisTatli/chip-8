#include <SDL2/SDL.h>
#include "emulator.h"
#include "chip8.h"
#include "platform.h"

bool init_sdl(sdl_t *sdl, emu_context_t *emu){
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO);
    sdl->window = SDL_CreateWindow("chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED
                                   , emu->width * emu->pixel_scale 
                                   , emu->height * emu->pixel_scale
                                   , 0);
    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, 0);
    
    sdl->desired = (SDL_AudioSpec){
        .freq = 44100,
        .format = AUDIO_S16LSB,
        .channels = 1,
        .samples = 512,
        .callback = audio_callback,
        .userdata = &emu->audio,
    };
    if(SDL_OpenAudio(&sdl->desired, &sdl->obtained) < 0){
        SDL_Log("Error: Could not open audio\n");
    }
    emu->audio.samples_per_sec = sdl->obtained.freq;
    return true;
}

void audio_callback(void *userdata, uint8_t *stream, int len) {
    //https://davidgow.net/handmadepenguin/ch8.html
    struct audio_info *info = (struct audio_info*)userdata;
    int16_t *audio_data = (int16_t *)stream;
    const int32_t square_wave_period = info->samples_per_sec/ info->tone_hz;
    const int32_t half_square_wave_period = square_wave_period / 2;

    for (int i = 0; i < len / 2; i++)
        audio_data[i] = ((info->running_sample_index++ / half_square_wave_period) % 2) ? 
                        info->tone_volume: 
                        -info->tone_volume;
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
void process_input(emu_context_t* emu, chip8_context_t* chip8){
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0){

        if( e.type == SDL_QUIT ) 
        {
            emu->state = QUIT;
        }
        if(e.type == SDL_KEYDOWN){
            switch (e.key.keysym.sym) {
                case SDLK_n:{
                    emu->state = STEP;
                } break;

                case SDLK_1:{chip8->keypad[0x1] = true; SDL_Log("1 Pressed\n");} break;
                case SDLK_2:{chip8->keypad[0x2] = true;} break;
                case SDLK_3:{chip8->keypad[0x3] = true;} break;
                case SDLK_4:{chip8->keypad[0xC] = true;} break;
                case SDLK_q:{chip8->keypad[0x4] = true;} break;
                case SDLK_w:{chip8->keypad[0x5] = true;} break;
                case SDLK_e:{chip8->keypad[0x6] = true;} break;
                case SDLK_r:{chip8->keypad[0xD] = true;} break;
                case SDLK_a:{chip8->keypad[0x7] = true;} break;
                case SDLK_s:{chip8->keypad[0x8] = true;} break;
                case SDLK_d:{chip8->keypad[0x9] = true;} break;
                case SDLK_f:{chip8->keypad[0xE] = true;} break;
                case SDLK_z:{chip8->keypad[0xA] = true;} break;
                case SDLK_x:{chip8->keypad[0x0] = true;} break;
                case SDLK_c:{chip8->keypad[0xB] = true;} break;
                case SDLK_v:{chip8->keypad[0xF] = true;} break;
            }
        }
        if(e.type == SDL_KEYUP){
            switch (e.key.keysym.sym) {
                
                case SDLK_1:{chip8->keypad[0x1] = false;} break;
                case SDLK_2:{chip8->keypad[0x2] = false;} break;
                case SDLK_3:{chip8->keypad[0x3] = false;} break;
                case SDLK_4:{chip8->keypad[0xC] = false;} break;
                case SDLK_q:{chip8->keypad[0x4] = false;} break;
                case SDLK_w:{chip8->keypad[0x5] = false;} break;
                case SDLK_e:{chip8->keypad[0x6] = false;} break;
                case SDLK_r:{chip8->keypad[0xD] = false;} break;
                case SDLK_a:{chip8->keypad[0x7] = false;} break;
                case SDLK_s:{chip8->keypad[0x8] = false;} break;
                case SDLK_d:{chip8->keypad[0x9] = false;} break;
                case SDLK_f:{chip8->keypad[0xE] = false;} break;
                case SDLK_z:{chip8->keypad[0xA] = false;} break;
                case SDLK_x:{chip8->keypad[0x0] = false;} break;
                case SDLK_c:{chip8->keypad[0xB] = false;} break;
                case SDLK_v:{chip8->keypad[0xF] = false;} break;
            }
        }
    }
}
