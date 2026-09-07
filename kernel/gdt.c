#include "gdt.h"

struct gdt_entry { uint16_t limit_low, base_low; uint8_t base_mid, access, gran, base_high; } __attribute__((packed));
struct gdt_ptr { uint16_t limit; uint32_t base; } __attribute__((packed));

static struct gdt_entry gdt[7];
static struct gdt_ptr gp;

static void set_gate(int n, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[n].base_low = base & 0xFFFF;
    gdt[n].base_mid = (base >> 16) & 0xFF;
    gdt[n].base_high = (base >> 24) & 0xFF;
    gdt[n].limit_low = limit & 0xFFFF;
    gdt[n].gran = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[n].access = access;
}

void gdt_set_tss(uint32_t base, uint32_t limit) {
    set_gate(5, base, limit, 0x89, 0x00);
    gdt[6].base_low = (base >> 16) & 0xFFFF;
    gdt[6].base_mid = (base >> 24) & 0xFF;
    gdt[6].base_high = 0;
    gdt[6].limit_low = 0;
    gdt[6].gran = 0;
    gdt[6].access = 0;
}

void gdt_init(void) {
    gp.limit = sizeof(gdt) - 1;
    gp.base = (uint32_t)&gdt;
    set_gate(0, 0, 0, 0, 0);
    set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    __asm__ volatile ("lgdt %0" : : "m"(gp));
    __asm__ volatile (
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%ss\n"
        "ljmp $0x08, $1f\n"
        "1:\n" : : : "ax", "memory");
}

void enter_user_mode(uint32_t entry, uint32_t stack) {
    __asm__ volatile (
        "cli\n"
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "pushl $0x23\n"
        "pushl %[stack]\n"
        "pushfl\n"
        "orl $0x200, (%%esp)\n"
        "pushl $0x1B\n"
        "pushl %[entry]\n"
        "iret\n"
        : : [entry] "r"(entry), [stack] "r"(stack) : "ax", "memory");
    __builtin_unreachable();
}
