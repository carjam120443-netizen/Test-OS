#include <stdint.h>
#include "paging.h"

#define MAX_ADDRESS_SPACES 8
#define USER_PD_FLAG 0x007u
#define KERNEL_PD_FLAG 0x003u

static uint32_t page_directories[MAX_ADDRESS_SPACES][1024] __attribute__((aligned(4096)));
static uint32_t page_tables[MAX_ADDRESS_SPACES][4][1024] __attribute__((aligned(4096)));
static uint8_t space_used[MAX_ADDRESS_SPACES];
static uint32_t current_cr3;

static inline void write_cr3(uint32_t value) { __asm__ volatile ("mov %0, %%cr3" : : "r"(value) : "memory"); }
static inline uint32_t read_cr0(void) { uint32_t value; __asm__ volatile ("mov %%cr0, %0" : "=r"(value)); return value; }
static inline void write_cr0(uint32_t value) { __asm__ volatile ("mov %0, %%cr0" : : "r"(value) : "memory"); }

static void build_space(uint32_t slot) {
    for (uint32_t i = 0; i < 1024; ++i) page_directories[slot][i] = 0x00000002;
    for (uint32_t table = 0; table < 4; ++table) {
        for (uint32_t i = 0; i < 1024; ++i) {
            uint32_t address = table * 0x400000u + i * 0x1000u;
            /* Kernel 0-4 MiB is supervisor-only; userspace starts at 4 MiB. */
            page_tables[slot][table][i] = address | (table == 0 ? KERNEL_PD_FLAG : USER_PD_FLAG);
        }
        page_directories[slot][table] = ((uint32_t)page_tables[slot][table]) | USER_PD_FLAG;
    }
}

int paging_init(void) {
    for (uint32_t i = 0; i < MAX_ADDRESS_SPACES; ++i) space_used[i] = 0;
    space_used[0] = 1;
    build_space(0);
    current_cr3 = (uint32_t)page_directories[0];
    write_cr3(current_cr3);
    write_cr0(read_cr0() | 0x80000000u);
    return 1;
}

uint32_t paging_create_user_space(void) {
    for (uint32_t i = 1; i < MAX_ADDRESS_SPACES; ++i) {
        if (space_used[i]) continue;
        space_used[i] = 1;
        build_space(i);
        return (uint32_t)page_directories[i];
    }
    return 0;
}

int paging_switch(uint32_t page_directory) {
    if (!page_directory) return 0;
    current_cr3 = page_directory;
    write_cr3(page_directory);
    return 1;
}

uint32_t paging_current(void) { return current_cr3; }
