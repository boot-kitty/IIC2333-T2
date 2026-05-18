#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct homerFile
{
    int process_id;

    char file_name[15];

    uint64_t size;
    uint32_t virtual_address;

    int file_slot;

    char mode;

    bool exists;
} 
homerFile;