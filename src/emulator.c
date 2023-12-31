#include <SDL2/SDL_audio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "chip8.h"
#include "emulator.h"
#include "SDL2/SDL_log.h"

void init_emu_context(emu_context_t *emu, char* filepath){
    *emu =(emu_context_t){
        .width = CHIP8_SCREEN_WIDTH,
        .height = CHIP8_SCREEN_HEIGHT,
        .bg_color = 0x00000000,
        .fg_color = 0x00FF0000,
        .clock_speed = 700, //standard speed fits most chip8 roms
        .pixel_scale = 20,
        .state = RUNNING,
        .audio.tone_hz = 440,
        .audio.tone_volume = 3000,
        .audio.samples_per_sec = 44100,
        .audio.running_sample_index = 0,
    };

    if(!load_rom(filepath, emu)){
        SDL_Log("Failed loading rom\n");
    }
}

bool load_rom(char *rom_path, emu_context_t *emu){
    emu->rom = fopen(rom_path, "rb");
    if(!emu->rom){
        SDL_Log("Failed loading %s, file is invalid or does not exist.\n", rom_path);
        return false;
    }
    
    fseek(emu->rom, 0, SEEK_END);
    emu->rom_size = ftell(emu->rom);
    rewind(emu->rom);

    uint32_t max_rom_size = 4096 - 0x200; //TODO make this better

    if(emu->rom_size > max_rom_size){
        SDL_Log("Failed loading %s, ROM size exceeds %d bytes.\n", rom_path, max_rom_size);
        return false;
    }

    return true;
}
