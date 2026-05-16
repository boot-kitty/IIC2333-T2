#pragma once
#include "../homer_File/homer_File.h"

/* ====== FUNCIONES GENERALES ====== */

void mount_memory(char* memory_path);

void list_processes();

int processes_slots();

void list_files(int process_id);

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
