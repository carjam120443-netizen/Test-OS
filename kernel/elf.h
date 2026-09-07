#ifndef TEST_OS_ELF_H
#define TEST_OS_ELF_H

#include <stdint.h>

struct elf_load_result { uint32_t entry; uint32_t image_start; uint32_t image_end; };

int elf_load(const uint8_t *image, uint32_t size, struct elf_load_result *result);

#endif
