#include "homer_memory_API.h"
#include "../homer_File/homer_File.h"

static char* path;

/* ====== FUNCIONES AUXILIARES ====== */

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

static long get_pcb_offset(int slot)
{
    return PCB_TABLE_START + slot * PCB_SIZE;
}

static int find_process_slot(FILE* memory, int process_id)
{
    unsigned char pcb[PCB_SIZE];

    fseek(memory, PCB_TABLE_START, SEEK_SET);

    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        fread(pcb, sizeof(unsigned char), PCB_SIZE, memory);

        if ((pcb[0] & PROCESS_EXISTS) && pcb[15] == process_id)
        {
            return i;
        }
    }

    return -1;
}

// static uint32_t paddr_rel(uint16_t pfn, uint16_t offset)
// {
//     return ((uint32_t)pfn << 15) | offset;
// }

// static uint64_t paddr_abs(uint32_t paddr)
// {
//     return ((uint64_t)DATA_START) + paddr;
// }

static uint32_t read_ipt_entry(FILE* memory, int pfn)
{
    uint8_t bytes[3];

    long offset = IPT_START + pfn * 3;

    fseek(memory, offset, SEEK_SET);
    fread(bytes, sizeof(uint8_t), 3, memory);

    return bytes[0]
         | (bytes[1] << 8)
         | (bytes[2] << 16);
}

static void write_ipt_entry(FILE* memory, int pfn, uint32_t value)
{
    uint8_t bytes[3];

    bytes[0] = value & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;

    long offset = IPT_START + pfn * 3;

    fseek(memory, offset, SEEK_SET);
    fwrite(bytes, sizeof(uint8_t), 3, memory);
}

// static int find_pfn(FILE* memory, int process_id, uint16_t vpn)
// {
//     for (int pfn = 0; pfn < TOTAL_FRAMES; pfn++)
//     {
//         uint32_t entry = read_ipt_entry(memory, pfn);
//         int valid = entry & 1;
//         int pid = (entry >> 1) & 0x3FF;
//         int entry_vpn = (entry >> 11) & 0x1FFF;
//         if (valid && pid == process_id && entry_vpn == vpn)
//         {
//             return pfn;
//         }
//     }

//     return -1;
// }

static int finish_process_by_slot(FILE* memory, int slot)
{
    unsigned char pcb[PCB_SIZE];
    fseek(memory, get_pcb_offset(slot), SEEK_SET);
    fread(pcb, sizeof(unsigned char), PCB_SIZE, memory);

    if (!(pcb[0] & PROCESS_EXISTS))
    {
        return -1;
    }
    int process_id = pcb[15];
    // Liberar frames asignados al proceso en IPT, y liberar frames en el bitmap
    for (int frame = 0; frame < TOTAL_FRAMES; frame++) 
    {
        uint32_t entry = read_ipt_entry(memory, frame); // 3 bytes por entrada
        int valid = entry >> 23;
        uint8_t pid = (entry >> 13) & 0xFF;
        if (valid && pid == process_id) {
            // printf("Liberando frame %d asignado al proceso %d\n", frame, process_id);
            // Marcar frame como libre en el bitmap
            long bitmap_byte_pos = BITMAP_START + (frame / 8);
            uint8_t bitmap_byte;
            fseek(memory, bitmap_byte_pos, SEEK_SET);
            fread(&bitmap_byte, sizeof(uint8_t), 1, memory);
            uint8_t new_bitmap_byte = bitmap_byte;
            new_bitmap_byte &= ~(1 << (7 - (frame % 8))); // Marcar bit como 0
            fseek(memory, bitmap_byte_pos, SEEK_SET);
            fwrite(&new_bitmap_byte, sizeof(uint8_t), 1, memory);
        }
        entry &= ~(1 << 23); // Marcar entrada como inválida
        write_ipt_entry(memory, frame, entry);
    }
    
    // Invalidar entradas de la tabla de archivos
    for (int i = 0; i < MAX_FILES; i++) 
    {
        int file_offset = FILE_TABLE_START + i * FILE_ENTRY_SIZE;
        pcb[file_offset] &= ~ENTRY_VALID; // Marcar entrada como inválida
    }
    // Invalidar PCB
    pcb[0] &= ~PROCESS_EXISTS; // Marcar proceso como inexistente

    // Sobreescribir PCB en memoria
    fseek(memory, get_pcb_offset(slot), SEEK_SET);
    fwrite(pcb, sizeof(unsigned char), PCB_SIZE, memory);

    return 0;
}

/* ====== FUNCIONES GENERALES ====== */

void mount_memory(char* memory_path)
{
    path = memory_path;
}

void list_processes() 
{
    FILE* memory = fopen(path, "rb");
    if (memory == NULL) {
        perror("Error al abrir el archivo de memoria");
        return;
    }
    
    // Leer la tabla de procesos
    fseek(memory, PCB_TABLE_START, SEEK_SET);

    for (int i = 0; i < MAX_PROCESSES; i++) 
    {
        unsigned char pcb[PCB_SIZE];

        fread(pcb,sizeof(unsigned char), PCB_SIZE, memory);

        if (pcb[0] & PROCESS_EXISTS) 
        {
            char process_name[PROCESS_NAME_SIZE + 1];
            memcpy(process_name, &pcb[1], PROCESS_NAME_SIZE);
            process_name[PROCESS_NAME_SIZE] = '\0';
            int process_id = pcb[15];
            printf("%d\t%s\n", process_id, process_name);
        }
    }

    fclose(memory);
}

int processes_slots() 
{
    FILE* memory = fopen(path, "rb");
    if (memory == NULL) 
    {
        perror("Error al abrir el archivo de memoria");
        return -1;
    }
    
    int count = 0;
    fseek(memory, PCB_TABLE_START, SEEK_SET);
    for (int i = 0; i < MAX_PROCESSES; i++) 
    {
        unsigned char pcb[PCB_SIZE];
        fread(pcb, sizeof(unsigned char), PCB_SIZE, memory);

        if (!(pcb[0] & PROCESS_EXISTS)) 
        {
            count++;
        }
    }
    fclose(memory);
    return count;
}

void list_files(int process_id) {
    FILE* memory = fopen(path, "rb");
    if (memory == NULL) 
    {
        perror("Error al abrir el archivo de memoria");
        return;
    }
    
    // Buscar el PCB del proceso
    int process_slot = find_process_slot(memory, process_id);
    if (process_slot == -1)
    {
        printf("Proceso con ID %d no encontrado.\n", process_id);
        fclose(memory);
        return;
    }
    unsigned char pcb[PCB_SIZE];
    fseek(memory, get_pcb_offset(process_slot), SEEK_SET);
    fread(pcb, sizeof(unsigned char), PCB_SIZE, memory);
    fclose(memory);
    
    // Leer la tabla de archivos del proceso
    // [valid][name(14B)][size(5B)][v_addr(4B)]
    for (int j = 0; j < MAX_FILES; j++) 
    {
        int file_entry_offset = FILE_TABLE_START + j * FILE_ENTRY_SIZE;
        if (pcb[file_entry_offset] & ENTRY_VALID) 
        {
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
            // DEBUG:
            // uint32_t offset = v_addr & 0x7FFF;  // 0b111 1111 1111 1111
            // uint64_t paddr = paddr_abs(paddr_rel(vpn, offset));
            // printf("    Dirección física: 0x%lX\n", paddr);
            
        }
    }
    
}

void frame_bitmap_status() 
{
    FILE* memory = fopen(path, "rb");
    if (memory == NULL) 
    {
        perror("Error al abrir el archivo de memoria");
        return;
    }
    
    fseek(memory, BITMAP_START, SEEK_SET);
    uint64_t bitmap[BITMAP_SIZE / sizeof(uint64_t)];
    fread(bitmap, sizeof(uint64_t), BITMAP_SIZE / sizeof(uint64_t), memory);
    fclose(memory);

    int used_frames = 0;
    for (int i = 0; i < BITMAP_SIZE / sizeof(uint64_t); i++) 
    {
        used_frames += count_set_bits_64(bitmap[i]);
    }
    printf("USADOS: %d\tLIBRES: %d\n", used_frames, TOTAL_FRAMES - used_frames);
}

int format_memory(char* memory_path) 
{
    FILE* memory = fopen(memory_path, "wb");
    if (memory == NULL) 
    {
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

int start_process(int process_id, char* process_name)
{
    FILE* memory = fopen(path, "rb+");

    if (memory == NULL)
    {
        perror("Error al abrir el archivo de memoria");
        return -1;
    }

    // Verificar si el PID ya existe
    if (find_process_slot(memory, process_id) != -1)
    {
        printf("Error: El proceso con ID %d ya existe.\n", process_id);
        fclose(memory);
        return -1;
    }

    int free_slot = -1;
    unsigned char pcb[PCB_SIZE];

    fseek(memory, PCB_TABLE_START, SEEK_SET);

    // Buscar PCB libre
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        fread(pcb, sizeof(unsigned char), PCB_SIZE, memory);

        if (!(pcb[0] & PROCESS_EXISTS))
        {
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1)
    {
        printf("No hay slots de proceso disponibles.\n");
        fclose(memory);
        return -1;
    }

    // Inicializar PCB limpio
    memset(pcb, 0, PCB_SIZE);
    pcb[0] = PROCESS_EXISTS;
    strncpy((char*)&pcb[1], process_name, PROCESS_NAME_SIZE);
    pcb[15] = process_id;

    fseek(memory, get_pcb_offset(free_slot), SEEK_SET);
    fwrite(pcb, sizeof(unsigned char), PCB_SIZE, memory);

    fclose(memory);

    return 0;
}

int finish_process(int process_id)
{
    FILE* memory = fopen(path, "rb+");

    if (memory == NULL)
    {
        perror("Error al abrir el archivo de memoria");
        return -1;
    }

    int process_slot = find_process_slot(memory, process_id);

    if (process_slot == -1)
    {
        fclose(memory);
        return -1;
    }

    int result = finish_process_by_slot(memory, process_slot);
    if (result == 0) {
        printf("Proceso con ID %d finalizado exitosamente.\n", process_id);
    } else {
        fprintf(stderr, "Error al finalizar proceso %d en slot %d\n", process_id, process_slot);
    }
    fclose(memory);
    return result;
}

int clear_all_processes()
{
    FILE* memory = fopen(path, "rb+");

    if (memory == NULL)
    {
        perror("Error al abrir el archivo de memoria");
        return -1;
    }

    unsigned char pcb[PCB_SIZE];
    int processes_cleared = 0;

    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        fseek(memory, get_pcb_offset(i), SEEK_SET);
        fread(pcb, sizeof(unsigned char), PCB_SIZE, memory);

        if (pcb[0] & PROCESS_EXISTS)
        {
            if (finish_process_by_slot(memory, i) == 0) {
                printf("Proceso %d en slot %d limpiado exitosamente.\n", pcb[15], i);
                processes_cleared++;
            } else {
                fprintf(stderr, "Error al limpiar proceso %d en slot %d\n", pcb[15], i);
            }
        }
    }
    fclose(memory);
    return processes_cleared;
}

int file_table_slots(int process_id)
{
    FILE* memory = fopen(path, "rb");

    if (memory == NULL)
    {
        perror("Error al abrir el archivo de memoria");
        return -1;
    }

    int process_slot = find_process_slot(memory, process_id);

    if (process_slot == -1)
    {
        fclose(memory);
        return -1;
    }

    unsigned char pcb[PCB_SIZE];
    fseek(memory, get_pcb_offset(process_slot), SEEK_SET);
    fread(pcb, sizeof(unsigned char), PCB_SIZE, memory);
    fclose(memory);

    int free_slots = 0;

    for (int i = 0; i < MAX_FILES; i++)
    {
        int file_offset = FILE_TABLE_START + i * FILE_ENTRY_SIZE;

        if (!(pcb[file_offset] & ENTRY_VALID))
        {
            free_slots++;
        }
    }

    return free_slots;
}


/* ====== FUNCIONES PARA ARCHIVOS ====== */

homerFile* open_file(int process_id, char* file_name, char mode)
{
    FILE* memory = fopen(path, "rb");

    if (memory == NULL)
    {
        perror("Error al abrir memoria");
        return NULL;
    }

    int process_slot = find_process_slot(memory, process_id);

    if (process_slot == -1)
    {
        fclose(memory);
        return NULL;
    }

    unsigned char pcb[PCB_SIZE];

    fseek(memory, get_pcb_offset(process_slot), SEEK_SET);
    fread(pcb, sizeof(unsigned char), PCB_SIZE, memory);
    homerFile* file = malloc(sizeof(homerFile));

    if (file == NULL)
    {
        fclose(memory);
        return NULL;
    }

    memset(file, 0, sizeof(homerFile));

    file->process_id = process_id;
    file->mode = mode;

    strncpy(file->file_name, file_name, FILE_NAME_SIZE);
    file->file_name[14] = '\0';

    for (int i = 0; i < MAX_FILES; i++)
    {
        int offset = FILE_TABLE_START + i * FILE_ENTRY_SIZE;

        if (!(pcb[offset] & ENTRY_VALID))
        {
            continue;
        }

        char existing_name[15];

        memcpy(existing_name, &pcb[offset + 1], FILE_NAME_SIZE);

        existing_name[14] = '\0';

        if (strcmp(existing_name, file_name) == 0)
        {
            // Escritura
            if (mode == 'w')
            {
                free(file);
                fclose(memory);
                return NULL;
            }

            file->exists = true;
            file->file_slot = i;

            fclose(memory);
            return file;
        }
    }

    fclose(memory);

    // Lectura
    if (mode == 'r')
    {
        free(file);
        return NULL;
    }

    // Escritura en archivo nuevo
    return file;
}

// int read_file(homerFile* file desc, char* dest);

// int write_file(homerFile* file desc, char* src);

// void delete_file(int process id, char* file name);

void close_file(homerFile* file_desc)
{
    if (file_desc != NULL)
    {
        free(file_desc);
    }
}


/*====== BONUS =====*/

// void memory_report(char* output_path);
