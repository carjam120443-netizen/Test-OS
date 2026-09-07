#ifndef TEST_OS_FS_H
#define TEST_OS_FS_H

#include <stdint.h>

#define FS_MAX_FILES 32
#define FS_NAME_MAX 64

struct fs_file {
    char name[FS_NAME_MAX];
    const uint8_t *data;
    uint32_t size;
};

int fs_init(const uint8_t *image, uint32_t size);
const struct fs_file *fs_open(const char *path);
const uint8_t *fs_data(const struct fs_file *file);
uint32_t fs_size(const struct fs_file *file);
uint32_t fs_count(void);

#endif
