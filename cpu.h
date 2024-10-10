#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "memory.h"

typedef struct CPU
{
    // Registers
    union
    {
        uint16_t af;
        struct
        {
            union
            {
                uint8_t f;
                struct
                {
                    unsigned char : 4;
                    bool carry : 1;
                    bool half_carry : 1;
                    bool n : 1;
                    bool z : 1;
                };
            };
            uint8_t a;
        };
    };
    union
    {
        uint16_t bc;
        struct
        {
            uint8_t c;
            uint8_t b;
        };
    };
    union
    {
        uint16_t de;
        struct
        {
            uint8_t e;
            uint8_t d;
        };
    };
    union
    {
        uint16_t hl;
        struct
        {
            uint8_t l;
            uint8_t h;
        };
    };
    uint16_t sp;
    uint16_t pc;
    Memory* mem;
} CPU;

CPU* make_cpu(Memory* mem);
int execute_inst(CPU* cpu);
void print_reg(CPU* cpu);