#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
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

#endif // !CHIP8_H
