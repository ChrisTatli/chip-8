#include "chip8.h"
#include "emulator.h"
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_log.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

void decrement_timers(chip8_context_t *chip8){
    if(chip8->DT > 0){
        chip8->DT--;
    }
    if(chip8->ST > 0){
        chip8->ST--;
        SDL_PauseAudio(0);
    } else {
        SDL_PauseAudio(1);
    }
}

void execute_instr(chip8_context_t *chip8, instr_t *instr){
    //debug_print_instr(instr);
    bool carry = false;
    uint8_t flag = 0;
    switch (instr->first) {
        case 0x0:{
            if(instr->nnn == 0x0E0){
                memset(&chip8->display_buffer[0], 0, sizeof(chip8->display_buffer));
            }
            else if (instr->nnn == 0x0EE) {
                chip8->PC = chip8->stack[--chip8->SP];//HMMM
            }
        } break;

        case 0x1:{
            chip8->PC = instr->nnn;
        } break;

        case 0x2:{
            chip8->stack[chip8->SP++] = chip8->PC;
            chip8->PC = instr->nnn;
        } break;

        case 0x3:{
            if(chip8->V[instr->x] == instr->nn){
                chip8->PC += 2;
            }
        } break;
        
        case 0x4:{
            if(chip8->V[instr->x] != instr->nn){
                chip8->PC += 2;
            }
        } break;

        case 0x5:{
            if(chip8->V[instr->x] == chip8->V[instr->y]){
                chip8->PC += 2;
            }
        } break;

        case 0x6:{
            chip8->V[instr->x] = instr->nn;
        } break;

        case 0x7:{
            chip8->V[instr->x] += instr->nn;
        } break;
        
        case 0x8:{
            switch (instr->n) {
                case 0x0:{
                    chip8->V[instr->x] = chip8->V[instr->y];
                } break;
                case 0x1:{
                    chip8->V[instr->x] |= chip8->V[instr->y];
                    chip8->V[0xF] = 0;
                } break;
                case 0x2:{
                    chip8->V[instr->x] &= chip8->V[instr->y];
                    chip8->V[0xF] = 0;
                } break;
                case 0x3:{
                    chip8->V[instr->x] ^= chip8->V[instr->y];
                    chip8->V[0xF] = 0;
                } break;
                case 0x4:{
                    uint16_t res = chip8->V[instr->x] + chip8->V[instr->y];
                    chip8->V[instr->x] = res & 0xFF;
                    chip8->V[0xF] = res > 255;
                } break;
                case 0x5:{
                    flag =  chip8->V[instr->x] >= chip8->V[instr->y]; 
                    chip8->V[instr->x] -= chip8->V[instr->y];                                
                    chip8->V[0xF] = flag;
                } break;
                case 0x6:{
                    chip8->V[instr->x] = chip8->V[instr->y];
                    flag = chip8->V[instr->x] & 1; 
                    chip8->V[instr->x] >>= 1;
                    chip8->V[0xF] = flag; 
                } break;
                case 0x7:{
                    flag = chip8->V[instr->y] >= chip8->V[instr->x];
                    chip8->V[instr->x] = chip8->V[instr->y] - chip8->V[instr->x];
                    chip8->V[0xF] = flag;
                } break;
                case 0xE:{
                    chip8->V[instr->x] = chip8->V[instr->y];
                    flag = (chip8->V[instr->x] >> 7) & 1;
                    chip8->V[instr->x] <<= 1;
                    chip8->V[0xF] = flag;
                } break;
            }
        } break;

        case 0x9:{
            if(chip8->V[instr->x] != chip8->V[instr->y]){
                chip8->PC += 2;
            }

        } break;

        case 0xA:{
            chip8->I = instr->nnn; 
        } break;

        case 0xB:{
            chip8->PC = instr->nnn + chip8->V[0];
        } break;

        case 0xC:{
            chip8->V[instr->x] = rand() & instr->nn;
        } break;
        
        case 0xD:{
            uint8_t x = chip8->V[instr->x] & CHIP8_SCREEN_WIDTH - 1; 
            uint8_t y = chip8->V[instr->y] & CHIP8_SCREEN_HEIGHT - 1;
            uint8_t n = instr->n;
            chip8->V[0xF] = 0;


            for (uint8_t row = 0; row < n; ++row) {
                if((row + y) >= CHIP8_SCREEN_HEIGHT) break;
                uint8_t sprite_data = chip8->ram[chip8->I + row];
                for(uint8_t col = 0; col < 8 ; ++col){
                    if((col + x) >= CHIP8_SCREEN_WIDTH) break;
                    if(!(sprite_data >> (7 - col) & 0x1)) continue;
                    uint16_t px_pos = (x + col) + (y + row)*CHIP8_SCREEN_WIDTH;
                    if(chip8->display_buffer[px_pos]){
                        chip8->V[0xF] = 1;    
                    }
                    chip8->display_buffer[px_pos] ^= 1;
                }
            }
            //SDL_Log("DXYN: Drew sprite of height %u at %u, %u\n", instr->n, chip8->V[instr->x], chip8->V[instr->y]);
        } break;

        case 0xE:{
            if (instr->nn == 0x9E) {
               if(chip8->keypad[chip8->V[instr->x]]){
                    chip8->PC += 2;
               }  
            }
            if (instr->nn == 0xA1){
                if(!chip8->keypad[chip8->V[instr->x]]){
                    chip8->PC += 2;
                }
            }
        } break;

        case 0xF:{
            switch (instr->nn) {
                case 0x07:{
                    chip8->V[instr->x] = chip8->DT;
                } break;

                case 0x0A:{
                    static bool key_pressed = false;
                    static uint8_t key = 0;
                    for(int8_t i = 0; i < 16; i ++){
                        if(chip8->keypad[i]){
                            key = i;
                            key_pressed = true;
                            break;
                        }
                    }
                    if(!key_pressed){
                        chip8->PC -= 2;
                    } else {
                        if(chip8->keypad[key]){
                            chip8->PC -= 2;
                        } else {
                            chip8->V[instr->x] = key;
                            key_pressed = false;
                        }
                    }


                } break;

                case 0x15: {
                    chip8->DT = chip8->V[instr->x];
                } break;
                case 0x18:{
                    chip8->ST = chip8->V[instr->x];
                } break;
                case 0x1E:{
                    chip8->I += chip8->V[instr->x];
                } break;
                case 0x29:{
                    chip8->I = 5 * chip8->V[instr->x];
                } break;
                case 0x33:{
                    uint8_t BCD = chip8->V[instr->x];
                    chip8->ram[chip8->I] = BCD / 100;
                    chip8->ram[chip8->I + 1] = (BCD / 10) % 10;
                    chip8->ram[chip8->I + 2] = BCD % 10;
                } break;
                case 0x55:{
                    for (uint8_t i = 0; i <= instr->x; i++) {
                        chip8->ram[chip8->I] = chip8->V[i];
                        chip8->I++;
                    }
                } break;
                case 0x65:{
                    for (uint8_t i = 0; i <= instr->x; i++) {
                        chip8->V[i] = chip8->ram[chip8->I];
                        chip8->I++;
                    }
                } break;
            }
        } break;
    }    
}
