#ifndef TEST_OS_GDT_H
#define TEST_OS_GDT_H
#include <stdint.h>
void gdt_init(void);
void gdt_set_tss(uint32_t base, uint32_t limit);
void enter_user_mode(uint32_t entry, uint32_t stack);
#endif
