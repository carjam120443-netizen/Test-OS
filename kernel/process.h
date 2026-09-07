#ifndef TEST_OS_PROCESS_H
#define TEST_OS_PROCESS_H

#include <stdint.h>

#define TESTOS_MAX_PROCESSES 16

#define PROCESS_UNUSED 0
#define PROCESS_READY  1
#define PROCESS_RUNNING 2
#define PROCESS_SLEEPING 3
#define PROCESS_ZOMBIE 4

struct process {
    uint32_t pid;
    uint32_t parent_pid;
    uint32_t state;
    uint32_t entry;
    uint32_t user_stack;
    uint32_t image_start;
    uint32_t image_end;
    const char *name;
};

void process_init(void);
int process_create(const char *name, uint32_t entry, uint32_t image_start, uint32_t image_end);
const struct process *process_get(uint32_t pid);
uint32_t process_count(void);
const struct process *process_table(void);

#endif
