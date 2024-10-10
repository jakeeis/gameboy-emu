#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct Memory
{
    uint8_t ram[0xffff];
} Memory;

Memory* make_memory();
