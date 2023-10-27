#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>
#include "emulator.h"

typedef struct {
    //Memory
    uint8_t ram[4096];

    //Registers
    uint8_t V[16]; //16 general purpose 8 bit regs

    uint16_t I; //Stores mem addrees 16 bit
    uint16_t PC; //Program Counter
    uint8_t SP; //Stack Pointer
    uint8_t delay_reg; // Delay timer
    uint8_t sound_reg; // Sound timer

    uint16_t stack[16];

    //Display
    uint8_t display_buffer[64 * 32];
    
} chip8_context_t;

typedef struct{
    uint16_t opcode;
    uint8_t first;
    uint16_t nnn;
    uint8_t nn;
    uint8_t n;
    uint8_t x;
    uint8_t y;
}instr_t;


void debug_print_instr(instr_t*, chip8_context_t *);
instr_t fetch_instr(chip8_context_t *);
void execute_instr(chip8_context_t *, instr_t *);
bool init_chip8(emu_context_t *, chip8_context_t *);

#endif // !CHIP8_H
