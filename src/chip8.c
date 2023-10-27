#include "chip8.h"
#include "emulator.h"
#include <SDL2/SDL_log.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


static uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

bool init_chip8(emu_context_t *emu, chip8_context_t *chip8){
    const uint32_t entry_point = 0x200;
    
    memset(chip8, 0, sizeof(chip8_context_t));
    
    memmove(&chip8->ram[0], font, sizeof(font));

    if(fread(&chip8->ram[entry_point], 1, emu->rom_size, emu->rom) != emu->rom_size){
        SDL_Log("Failed initialising rom in chip8 memory.\n");
        return false;
    }

    //probably need to close rom file here or somewhere else


    chip8->PC = entry_point;
    return true;
}

instr_t fetch_instr(chip8_context_t *chip8){
    uint16_t opcode = chip8->ram[chip8->PC] << 8 | chip8->ram[chip8->PC+1];
    instr_t instr = {
        .opcode = opcode,
        .first = (opcode >> 12) & 0x0F, 
        .nnn = opcode & 0x0FFF,
        .nn = opcode & 0xFF,
        .n = opcode & 0x0F,
        .x = (opcode >> 8) & 0x0F,
        .y = (opcode >> 4) & 0x0F
    };
    chip8->PC += 2;
    return instr;
}

void debug_print_instr(instr_t *instr, chip8_context_t *chip8){
    SDL_Log("PC: %x, %4x     ", chip8->PC, instr->opcode);
    for(int i = 0; i <= 0xF; i++){
        SDL_Log("V%x: %3u     ", i, chip8->V[i]);
    }
    SDL_Log("\n\n");
}

void execute_instr(chip8_context_t *chip8, instr_t *instr){
    //debug_print_instr(instr);
    switch (instr->first) {
        case 0x0:{
            if(instr->n == 0x0){
                memset(&chip8->display_buffer[0], 0, sizeof(chip8->display_buffer));
            }
        } break;

        case 0x1:{
            chip8->PC = instr->nnn;
        } break;

        case 0x6:{
            chip8->V[instr->x] = instr->nn;
        } break;

        case 0x7:{
            chip8->V[instr->x] += instr->nn;
        } break;

        case 0xA:{
            chip8->I = instr->nnn; 
        } break;

        case 0xD:{
            uint8_t x = chip8->V[instr->x] % 64;
            uint8_t y = chip8->V[instr->y] % 32;
            chip8->V[0xF] = 0;
            for (uint8_t i = 0; i < instr->n; i++) {
                uint8_t sprite_data = chip8->ram[chip8->I + i];
                for(uint8_t j = 7; j >= 0; j--){
                    uint8_t check_bit = (sprite_data & (1 << j)); 
                    bool is_set = sprite_data & check_bit;
                    uint8_t *pixel = &chip8->display_buffer[y*64 + x];
                    if(check_bit && *pixel){
                        chip8->V[0xF] = 1;    
                    }
                    *pixel ^= check_bit;
                    if(++x >= 64) break;
                }
                if(++y >= 32) break;
            }
            //SDL_Log("DXYN: Drew sprite of height %u at %u, %u\n", instr->n, chip8->V[instr->x], chip8->V[instr->y]);
        } break;
    }    
}
