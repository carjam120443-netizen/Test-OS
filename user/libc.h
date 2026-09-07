#ifndef TEST_OS_LIBC_H
#define TEST_OS_LIBC_H

#include <stdint.h>
#include "syscall.h"

uint32_t strlen(const char *s);
void *memset(void *dst, int value, uint32_t n);
void *memcpy(void *dst, const void *src, uint32_t n);
int strcmp(const char *a, const char *b);
int puts(const char *s);

#endif
