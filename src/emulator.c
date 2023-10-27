#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "emulator.h"
#include "SDL2/SDL_log.h"

void init_emu_context(emu_context_t *emu){
    *emu =(emu_context_t){
        .width = 64,
        .height = 32,
        .bg_color = 0x00000000,
        .fg_color = 0x00FF0000,
        .clock_speed = 700, //standard speed fits most chip8 roms
        .pixel_scale = 20
    };

    if(!load_rom("/Users/christophertatli/dev/chip-8/roms/IBMLogo.ch8", emu)){
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
