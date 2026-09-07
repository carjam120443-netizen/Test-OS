#include <stdint.h>
#include "paging.h"

/* First 16 MiB are identity-mapped so the early kernel and user image can run. */
static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t page_tables[4][1024] __attribute__((aligned(4096)));

static inline void write_cr3(uint32_t value) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(value) : "memory");
}

static inline uint32_t read_cr0(void) {
    uint32_t value;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(value));
    return value;
}

static inline void write_cr0(uint32_t value) {
    __asm__ volatile ("mov %0, %%cr0" : : "r"(value) : "memory");
}

int paging_init(void) {
    for (uint32_t i = 0; i < 1024; ++i) page_directory[i] = 0x00000002;

    for (uint32_t table = 0; table < 4; ++table) {
        for (uint32_t i = 0; i < 1024; ++i) {
            /* Present + writable + user accessible during early bootstrap. */
            page_tables[table][i] = (table * 0x400000 + i * 0x1000) | 0x007;
        }
        page_directory[table] = ((uint32_t)page_tables[table]) | 0x007;
    }

    write_cr3((uint32_t)page_directory);
    write_cr0(read_cr0() | 0x80000000); /* CR0.PG */
    return 1;
}
