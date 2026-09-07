#include <stdint.h>
#include "tss.h"

struct tss32 {
    uint32_t prev_tss, esp0, ss0, esp1, ss1, esp2, ss2, cr3, eip, eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs, ldt;
    uint16_t trap, iomap;
} __attribute__((packed));

extern void gdt_set_tss(uint32_t base, uint32_t limit);
static struct tss32 tss __attribute__((aligned(16)));
static uint8_t kernel_stack[16384] __attribute__((aligned(16)));

void tss_init(void) {
    for (uint32_t i = 0; i < sizeof(tss); ++i) ((uint8_t*)&tss)[i] = 0;
    tss.ss0 = 0x10;
    tss.esp0 = (uint32_t)(kernel_stack + sizeof(kernel_stack));
    tss.iomap = sizeof(tss);
    gdt_set_tss((uint32_t)&tss, sizeof(tss) - 1);
    __asm__ volatile ("ltr %%ax" : : "a"((uint16_t)0x28));
}
