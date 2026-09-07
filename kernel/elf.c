#include "elf.h"

#define EI_NIDENT 16
#define PT_LOAD 1
#define EM_386 3
#define ELFCLASS32 1
#define ELFDATA2LSB 1

struct elf32_hdr { uint8_t ident[EI_NIDENT]; uint16_t type, machine; uint32_t version, entry, phoff, shoff, flags; uint16_t ehsize, phentsize, phnum, shentsize, shnum, shstrndx; } __attribute__((packed));
struct elf32_phdr { uint32_t type, offset, vaddr, paddr, filesz, memsz, flags, align; } __attribute__((packed));

static void memzero(uint8_t *p, uint32_t n) { for (uint32_t i = 0; i < n; ++i) p[i] = 0; }
static void memcpy8(uint8_t *d, const uint8_t *s, uint32_t n) { for (uint32_t i = 0; i < n; ++i) d[i] = s[i]; }

int elf_load(const uint8_t *image, uint32_t size, struct elf_load_result *result) {
    if (!image || size < sizeof(struct elf32_hdr) || !result) return 0;
    const struct elf32_hdr *h = (const struct elf32_hdr *)image;
    if (h->ident[0] != 0x7F || h->ident[1] != 'E' || h->ident[2] != 'L' || h->ident[3] != 'F') return 0;
    if (h->ident[4] != ELFCLASS32 || h->ident[5] != ELFDATA2LSB || h->machine != EM_386) return 0;
    if (h->phentsize < sizeof(struct elf32_phdr) || h->phnum == 0) return 0;
    if (h->phoff > size || h->phnum > (size - h->phoff) / h->phentsize) return 0;

    uint32_t low = 0xFFFFFFFFu, high = 0;
    for (uint32_t i = 0; i < h->phnum; ++i) {
        const struct elf32_phdr *p = (const struct elf32_phdr *)(image + h->phoff + i * h->phentsize);
        if (p->type != PT_LOAD) continue;
        if (p->filesz > p->memsz || p->offset > size || p->filesz > size - p->offset) return 0;
        if (p->vaddr < 0x00400000u || p->vaddr + p->memsz < p->vaddr || p->vaddr + p->memsz > 0x00F00000u) return 0;
        if (p->vaddr < low) low = p->vaddr;
        if (p->vaddr + p->memsz > high) high = p->vaddr + p->memsz;
    }
    if (low == 0xFFFFFFFFu || h->entry < low || h->entry >= high) return 0;

    for (uint32_t i = 0; i < h->phnum; ++i) {
        const struct elf32_phdr *p = (const struct elf32_phdr *)(image + h->phoff + i * h->phentsize);
        if (p->type != PT_LOAD) continue;
        memcpy8((uint8_t *)(uintptr_t)p->vaddr, image + p->offset, p->filesz);
        if (p->memsz > p->filesz) memzero((uint8_t *)(uintptr_t)(p->vaddr + p->filesz), p->memsz - p->filesz);
    }
    result->entry = h->entry;
    result->image_start = low;
    result->image_end = high;
    return 1;
}
