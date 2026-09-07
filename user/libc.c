#include "libc.h"

uint32_t strlen(const char *s) { uint32_t n = 0; if (!s) return 0; while (s[n]) ++n; return n; }

void *memset(void *dst, int value, uint32_t n) { uint8_t *d = (uint8_t *)dst; while (n--) *d++ = (uint8_t)value; return dst; }

void *memcpy(void *dst, const void *src, uint32_t n) { uint8_t *d = (uint8_t *)dst; const uint8_t *s = (const uint8_t *)src; while (n--) *d++ = *s++; return dst; }

int strcmp(const char *a, const char *b) { while (*a && *a == *b) { ++a; ++b; } return (uint8_t)*a - (uint8_t)*b; }

int puts(const char *s) { int r = sys_write(s); sys_write("\n"); return r; }
