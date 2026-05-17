#include "homer_memory_API.h"

static char* path;

// https://www.reddit.com/r/C_Programming/comments/jpt7dw/what_is_the_proper_way_to_count_bits_which_are/
#if defined(__GNUC__) || defined(__clang__)
    int count_set_bits_64(uint64_t n) {
        return __builtin_popcountll(n);
}
#else
    int count_set_bits_64(uint64_t n) {  // Brian Kernighan's algorithm
        int count = 0;
        for (count = 0; n; count++) {
            n &= (n - 1);
        }
        return count;
    }
#endif

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
    fclose(memory);
    
    if (!found) {
        printf("Proceso con ID %d no encontrado.\n", process_id);
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
    
}

void frame_bitmap_status() {
    FILE* memory = fopen(path, "rb");
    if (memory == NULL) {
        perror("Error al abrir el archivo de memoria");
        return;
    }
    
    fseek(memory, BITMAP_START, SEEK_SET);
    uint64_t bitmap[BITMAP_SIZE / sizeof(uint64_t)];
    fread(bitmap, sizeof(uint64_t), BITMAP_SIZE / sizeof(uint64_t), memory);
    fclose(memory);

    int used_frames = 0;
    for (int i = 0; i < BITMAP_SIZE / sizeof(uint64_t); i++) {
        used_frames += count_set_bits_64(bitmap[i]);
    }
    printf("Frames usados: %d, Frames libres: %d\n", used_frames, TOTAL_FRAMES - used_frames);
}

int format_memory(char* memory_path) {
    FILE* memory = fopen(memory_path, "wb");
    if (memory == NULL) {
        perror("Error al crear el archivo de memoria");
        return -1;
    }
    size_t total_size = PCB_TABLE_SIZE + IPT_SIZE + BITMAP_SIZE + DATA_SIZE;
    ftruncate(fileno(memory), 0);
    ftruncate(fileno(memory), total_size);
    fclose(memory);
    return 0;
}


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
