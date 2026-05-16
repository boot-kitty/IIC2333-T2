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

#define FRAME_SIZE          32768
#define TOTAL_FRAMES        65536

#define MAX_PROCESSES       32
#define PCB_SIZE            256

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

// void list_processes();

// int processes_slots();

// void list_files(int process_id);

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
