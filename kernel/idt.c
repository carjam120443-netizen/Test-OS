#include <stdint.h>
#include "idt.h"

struct idt_entry { uint16_t base_low, selector; uint8_t zero, flags; uint16_t base_high; } __attribute__((packed));
struct idt_ptr { uint16_t limit; uint32_t base; } __attribute__((packed));
static struct idt_entry idt[256];
static struct idt_ptr idtp;
extern void syscall_isr(void);

static void set_gate(uint8_t n, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[n].base_low = base & 0xFFFF;
    idt[n].base_high = (base >> 16) & 0xFFFF;
    idt[n].selector = selector;
    idt[n].zero = 0;
    idt[n].flags = flags;
}

void idt_init(void) {
    for (uint32_t i = 0; i < 256; ++i) set_gate((uint8_t)i, 0, 0x08, 0);
    set_gate(0x80, (uint32_t)syscall_isr, 0x08, 0xEE);
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint32_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idtp));
}
