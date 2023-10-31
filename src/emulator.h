#ifndef EMULATOR_H
#define EMULATOR_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef uint32_t color_t;

enum Emu_State{
    RUNNING,
    PAUSED,
    STEP,
    QUIT
};

typedef struct{
    uint32_t height;
    uint32_t width;
    color_t bg_color;
    color_t fg_color;
    uint32_t pixel_scale;
    uint32_t clock_speed;
    FILE *rom;
    uint32_t rom_size;
    enum Emu_State state;
} emu_context_t;


bool load_rom(char *, emu_context_t *);
void init_emu_context(emu_context_t *, char*);
#endif // !EMULATOR_H
