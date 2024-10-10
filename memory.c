#include "memory.h"

Memory* make_memory()
{
    Memory* mem = calloc(1, sizeof(Memory));
    return mem;
}