#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "cpu.h"
#include "test-runner.h"

int main(void)
{
    // Memory* mem = make_memory();
    // CPU* cpu = make_cpu(mem);

    // uint8_t rom[] = {
    //     0x00,
    //     0x03,
    //     0x13,
    //     0x23,
    //     0x33
    // };

    // memcpy(cpu->mem->ram, rom, sizeof(rom));
    // printf("%d\n", (int)sizeof(rom));

    // print_reg(cpu);

    // for (int i = 0; i < 10; i++)
    // {
    //     execute_inst(cpu);
    //     print_reg(cpu);
    // }

    for (int i = 0x00; i < 0x100; i++)
    {
        int numFailed = run_test(i);

        if (numFailed > 0)
        {
            printf("%d tests failed for opcode: 0x%02x\n", numFailed, i);
            break;
        }
    }

    for (int i = 0x00; i < 0x100; i++)
    {
        int numFailed = run_test(0xcb00 | i);

        if (numFailed > 0)
        {
            printf("%d tests failed for opcode: 0x%04x\n", numFailed, 0xcb00 | i);
            break;
        }
    }
}