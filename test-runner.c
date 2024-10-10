#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "test-runner.h"
#include "cJSON.h"

#define LOG_LEVEL 0

char* get_test_str(int fileIndex)
{
    FILE* file;
    char* buffer;
    long length;

    // Open file in read mode
    char* filename = calloc(100, sizeof(char));
    snprintf(filename, 100, "./GameboyCPUTests/v2/%02x.json", fileIndex);

    file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the buffer
    buffer = (char*)malloc(length + 1);
    if (buffer == NULL)
    {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    // Read file contents into buffer
    int size = fread(buffer, 1, length, file);
    buffer[length] = '\0'; // Null-terminate the string

    // Close the file
    fclose(file);
    free(filename);

    return buffer;
}

int run_test(int fileIndex)
{
    char* buffer = get_test_str(fileIndex);
    cJSON* json = cJSON_Parse(buffer);
    int numTests = cJSON_GetArraySize(json);

    int numFailed = 0;

    for (int i = 0; i < numTests; i++)
    {
#if LOG_LEVEL > 0
        printf("Testing 0x%02x:\t Test Number: %d\n", fileIndex, i);
#endif

        cJSON* testJson = cJSON_GetArrayItem(json, i);

        Memory* mem = make_memory();
        CPU* cpu = make_cpu(mem);

        cJSON* initial = cJSON_GetObjectItemCaseSensitive(testJson, "initial");
        cpu->a = cJSON_GetObjectItemCaseSensitive(initial, "a")->valueint;
        cpu->b = cJSON_GetObjectItemCaseSensitive(initial, "b")->valueint;
        cpu->c = cJSON_GetObjectItemCaseSensitive(initial, "c")->valueint;
        cpu->d = cJSON_GetObjectItemCaseSensitive(initial, "d")->valueint;
        cpu->e = cJSON_GetObjectItemCaseSensitive(initial, "e")->valueint;
        cpu->f = cJSON_GetObjectItemCaseSensitive(initial, "f")->valueint;
        cpu->h = cJSON_GetObjectItemCaseSensitive(initial, "h")->valueint;
        cpu->l = cJSON_GetObjectItemCaseSensitive(initial, "l")->valueint;
        cpu->pc = cJSON_GetObjectItemCaseSensitive(initial, "pc")->valueint - 1;
        cpu->sp = cJSON_GetObjectItemCaseSensitive(initial, "sp")->valueint;

        cJSON* ram = cJSON_GetObjectItemCaseSensitive(initial, "ram");
        int ramLength = cJSON_GetArraySize(ram);
        for (int j = 0; j < ramLength; j++)
        {
            cJSON* ramItem = cJSON_GetArrayItem(ram, j);
            mem->ram[cJSON_GetArrayItem(ramItem, 0)->valueint] = cJSON_GetArrayItem(ramItem, 1)->valueint;
        }

        int cycles = execute_inst(cpu);

        cJSON* final = cJSON_GetObjectItemCaseSensitive(testJson, "final");
        if (cpu->a != cJSON_GetObjectItemCaseSensitive(final, "a")->valueint)
        {
#if LOG_LEVEL > 1
            printf("\tIncorrect Value for a \t\t| Expected: 0x%02x;\t Actual: 0x%02x\n", cJSON_GetObjectItemCaseSensitive(final, "a")->valueint, cpu->a);
#endif
            numFailed++;
        }
        if (cpu->b != cJSON_GetObjectItemCaseSensitive(final, "b")->valueint)
        {
#if LOG_LEVEL > 1
            printf("\tIncorrect Value for b \t\t| Expected: 0x%02x;\t Actual: 0x%02x\n", cJSON_GetObjectItemCaseSensitive(final, "b")->valueint, cpu->b);
#endif
            numFailed++;
        }
        if (cpu->c != cJSON_GetObjectItemCaseSensitive(final, "c")->valueint)
        {
#if LOG_LEVEL > 1
            printf("\tIncorrect Value for c \t\t| Expected: 0x%02x;\t Actual: 0x%02x\n", cJSON_GetObjectItemCaseSensitive(final, "c")->valueint, cpu->c);
#endif
            numFailed++;
        }
        if (cpu->d != cJSON_GetObjectItemCaseSensitive(final, "d")->valueint)
        {
#if LOG_LEVEL > 1
            printf("\tIncorrect Value for d \t\t| Expected: 0x%02x;\t Actual: 0x%02x\n", cJSON_GetObjectItemCaseSensitive(final, "d")->valueint, cpu->d);
#endif
            numFailed++;
        }
        if (cpu->e != cJSON_GetObjectItemCaseSensitive(final, "e")->valueint)
        {
#if LOG_LEVEL > 1
            printf("\tIncorrect Value for e \t\t| Expected: 0x%02x;\t Actual: 0x%02x\n", cJSON_GetObjectItemCaseSensitive(final, "e")->valueint, cpu->e);
#endif
            numFailed++;
        }
        if (cpu->f != cJSON_GetObjectItemCaseSensitive(final, "f")->valueint)
        {
#if LOG_LEVEL > 1
            printf("\tIncorrect Value for f \t\t| Expected: 0x%02x;\t Actual: 0x%02x\n", cJSON_GetObjectItemCaseSensitive(final, "f")->valueint, cpu->f);
#endif
            numFailed++;
        }
        if (cpu->h != cJSON_GetObjectItemCaseSensitive(final, "h")->valueint)
        {
#if LOG_LEVEL > 1
            printf("\tIncorrect Value for h \t\t| Expected: 0x%02x;\t Actual: 0x%02x\n", cJSON_GetObjectItemCaseSensitive(final, "h")->valueint, cpu->h);
#endif
            numFailed++;
        }
        if (cpu->l != cJSON_GetObjectItemCaseSensitive(final, "l")->valueint)
        {
#if LOG_LEVEL > 1
            printf("\tIncorrect Value for l \t\t| Expected: 0x%02x;\t Actual: 0x%02x\n", cJSON_GetObjectItemCaseSensitive(final, "l")->valueint, cpu->l);
#endif
            numFailed++;
        }
        if (cpu->pc != cJSON_GetObjectItemCaseSensitive(final, "pc")->valueint - 1)
        {
#if LOG_LEVEL > 1
            printf("\tIncorrect Value for pc \t\t| Expected: 0x%04x;\t Actual: 0x%04x\n", cJSON_GetObjectItemCaseSensitive(final, "pc")->valueint - 1, cpu->pc);
#endif
            numFailed++;
        }
        if (cpu->sp != cJSON_GetObjectItemCaseSensitive(final, "sp")->valueint)
        {
#if LOG_LEVEL > 1
            printf("\tIncorrect Value for sp \t\t| Expected: 0x%04x;\t Actual: 0x%04x\n", cJSON_GetObjectItemCaseSensitive(final, "sp")->valueint, cpu->sp);
#endif
            numFailed++;
        }
        if (cycles != cJSON_GetArraySize(cJSON_GetObjectItemCaseSensitive(testJson, "cycles")))
        {
#if LOG_LEVEL > 1
            printf("\tIncorrect number of cycles \t| Expected: %d;\t\t Actual: %d\n", cJSON_GetArraySize(cJSON_GetObjectItemCaseSensitive(testJson, "cycles")), cycles);
#endif
            numFailed++;
        }

        cJSON* finalRam = cJSON_GetObjectItemCaseSensitive(final, "ram");
        int finalRamLength = cJSON_GetArraySize(finalRam);
        for (int j = 0; j < finalRamLength; j++)
        {
            cJSON* ramItem = cJSON_GetArrayItem(finalRam, j);

            int addr = cJSON_GetArrayItem(ramItem, 0)->valueint;
            int actualVal = mem->ram[addr];
            int expectedVal = cJSON_GetArrayItem(ramItem, 1)->valueint;

            if (actualVal != expectedVal)
            {
#if LOG_LEVEL > 1
                printf("\tIncorrect ram value at 0x%04x\t| Expected: %3d;\t Actual: %d\n", addr, expectedVal, actualVal);
#endif
                numFailed++;
            }
        }
    }

#if LOG_LEVEL > 0
    printf(numFailed == 0 ? "ALL TESTS PASS\n" : "%d TESTS FAILED\n", numFailed);
#endif

    free(buffer);
    cJSON_Delete(json);

    return numFailed;
}
