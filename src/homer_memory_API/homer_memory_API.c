#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include <string.h> //para strcmp
#include <stdbool.h> // bool, true, false
#include "homer_memory_API.h"

/* ====== Variables Globales ====== */

#define PCB_TABLE_SIZE      8192
#define IPT_SIZE            196608
#define BITMAP_SIZE         8192
#define DATA_SIZE           2147483648ULL

// [PCB Table][IPT][Bitmap][Data]
#define PCB_TABLE_START     0
#define IPT_START           (PCB_TABLE_START + PCB_TABLE_SIZE)
#define BITMAP_START        (IPT_START + IPT_SIZE)
#define DATA_START          (BITMAP_START + BITMAP_SIZE)

#define FRAME_SIZE          32768
#define TOTAL_FRAMES        65536

#define MAX_PROCESSES       32
#define PCB_SIZE            256
#define FILE_TABLE_START    16

#define MAX_FILES           10
#define FILE_ENTRY_SIZE     24

#define PROCESS_NAME_SIZE   14
#define FILE_NAME_SIZE      14

#define PROCESS_EXISTS      0x01
#define ENTRY_VALID         0x01

char* path;

/* ====================================== */

/* ====== FUNCIONES GENERALES ====== */

void mount_memory(char* memory_path)
{
    path = memory_path;
}

void list_processes() {
    FILE* memory = fopen(path, "rb");
    if (memory == NULL) {
        perror("Error al abrir el archivo de memoria");
        return;
    }
    
    // Leer la tabla de procesos
    fseek(memory, PCB_TABLE_START, SEEK_SET);
    for (int i = 0; i < PCB_TABLE_SIZE / PCB_SIZE; i++) {
        unsigned char pcb[PCB_SIZE];
        fread(pcb, sizeof(unsigned char), PCB_SIZE, memory);
        
        if (pcb[0] & PROCESS_EXISTS) {
            char process_name[PROCESS_NAME_SIZE + 1];
            memcpy(process_name, &pcb[1], PROCESS_NAME_SIZE);
            process_name[PROCESS_NAME_SIZE] = '\0';
            int process_id = pcb[15];
            printf("%d\t%s\n", process_id, process_name);
        }
    }
    fclose(memory);
}

int processes_slots() {
    FILE* memory = fopen(path, "rb");
    if (memory == NULL) {
        perror("Error al abrir el archivo de memoria");
        return -1;
    }
    
    int count = 0;
    fseek(memory, PCB_TABLE_START, SEEK_SET);
    for (int i = 0; i < PCB_TABLE_SIZE / PCB_SIZE; i++) {
        unsigned char pcb[PCB_SIZE];
        fread(pcb, sizeof(unsigned char), PCB_SIZE, memory);
        if (pcb[0] & PROCESS_EXISTS) {
            count++;
        }
    }
    fclose(memory);
    return count;
}

void list_files(int process_id) {
    FILE* memory = fopen(path, "rb");
    if (memory == NULL) {
        perror("Error al abrir el archivo de memoria");
        return;
    }
    
    // Buscar el PCB del proceso
    fseek(memory, PCB_TABLE_START, SEEK_SET);
    unsigned char pcb[PCB_SIZE];
    bool found = false;
    for (int i = 0; i < PCB_TABLE_SIZE / PCB_SIZE; i++) {
        fread(pcb, sizeof(unsigned char), PCB_SIZE, memory);
        if ((pcb[0] & PROCESS_EXISTS) && pcb[15] == process_id) {
            found = true;
            break;
        }
    }
    
    if (!found) {
        printf("Proceso con ID %d no encontrado.\n", process_id);
        fclose(memory);
        return;
    }
    
    // Leer la tabla de archivos del proceso
    // [valid][name(14B)][size(5B)][v_addr(4B)]
    for (int j = 0; j < MAX_FILES; j++) {
        int file_entry_offset = FILE_TABLE_START + j * FILE_ENTRY_SIZE;
        if (pcb[file_entry_offset] & ENTRY_VALID) {
            char file_name[FILE_NAME_SIZE + 1];
            unsigned long size = 0;
            unsigned int v_addr;
            memcpy(file_name, &pcb[file_entry_offset + 1], FILE_NAME_SIZE);
            file_name[FILE_NAME_SIZE] = '\0';
            memcpy(&size, &pcb[file_entry_offset + 1 + FILE_NAME_SIZE], 5);
            memcpy(&v_addr, &pcb[file_entry_offset + 1 + FILE_NAME_SIZE + 5], 4);
            int vpn = v_addr >> 15;
            // print VPN[hex] FILE_SIZE[dec] V_ADDR[hex] FILE_NAME
            printf("0x%X\t%lu\t0x%X\t%s\n", vpn, size, v_addr, file_name);
        }
    }
    
    fclose(memory);
}

// void frame_bitmap_status();

// int format_memory(char* memory path);



/* ====== FUNCIONES PARA PROCESOS ====== */

// int start_process(int process_id, char* process_name);

// int finish_process(int process_id);

// int clear_all_processes();

// int file_table_slots(int process_id);


/* ====== FUNCIONES PARA ARCHIVOS ====== */

// homerFile* open_file(int process_id, char* file_name, char mode);

// int read_file(homerFile* file desc, char* dest);

// int write_file(homerFile* file desc, char* src);

// void delete_file(int process id, char* file name);

// void close_file(homerFile* file_desc);


/*====== BONUS =====*/

// void memory_report(char* output_path);
