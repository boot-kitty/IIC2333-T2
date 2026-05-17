#pragma once

/* ====== Variables Globales ====== */

#define PCB_TABLE_SIZE      8192
#define IPT_SIZE            196608
#define BITMAP_SIZE         8192
#define DATA_SIZE           2147483648ULL

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

/* ====================================== */