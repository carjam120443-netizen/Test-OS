#include "fs.h"

#define RAMFS_MAGIC 0x31534654u /* TSF1 */

struct ramfs_header { uint32_t magic, count; } __attribute__((packed));
struct ramfs_entry { char name[FS_NAME_MAX]; uint32_t offset, size; } __attribute__((packed));

static struct fs_file files[FS_MAX_FILES];
static uint32_t file_count;

static int path_equal(const char *a, const char *b) {
    while (*a && *b && *a == *b) { ++a; ++b; }
    return *a == *b;
}

int fs_init(const uint8_t *image, uint32_t size) {
    file_count = 0;
    if (!image || size < sizeof(struct ramfs_header)) return 0;
    const struct ramfs_header *h = (const struct ramfs_header *)image;
    if (h->magic != RAMFS_MAGIC || h->count > FS_MAX_FILES) return 0;
    uint32_t table_end = sizeof(*h) + h->count * sizeof(struct ramfs_entry);
    if (table_end > size) return 0;

    const struct ramfs_entry *entries = (const struct ramfs_entry *)(image + sizeof(*h));
    for (uint32_t i = 0; i < h->count; ++i) {
        if (entries[i].offset > size || entries[i].size > size - entries[i].offset) return 0;
        for (uint32_t j = 0; j < FS_NAME_MAX; ++j) files[i].name[j] = entries[i].name[j];
        files[i].data = image + entries[i].offset;
        files[i].size = entries[i].size;
    }
    file_count = h->count;
    return 1;
}

const struct fs_file *fs_open(const char *path) {
    if (!path) return 0;
    for (uint32_t i = 0; i < file_count; ++i)
        if (path_equal(files[i].name, path)) return &files[i];
    return 0;
}

const uint8_t *fs_data(const struct fs_file *file) { return file ? file->data : 0; }
uint32_t fs_size(const struct fs_file *file) { return file ? file->size : 0; }
uint32_t fs_count(void) { return file_count; }
