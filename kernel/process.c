#include "process.h"

static struct process processes[TESTOS_MAX_PROCESSES];
static uint32_t next_pid = 1;
static uint32_t process_total = 0;

void process_init(void) {
    for (uint32_t i = 0; i < TESTOS_MAX_PROCESSES; ++i) {
        processes[i].pid = 0;
        processes[i].parent_pid = 0;
        processes[i].state = PROCESS_UNUSED;
        processes[i].entry = 0;
        processes[i].user_stack = 0;
        processes[i].image_start = 0;
        processes[i].image_end = 0;
        processes[i].page_directory = 0;
        processes[i].name = 0;
    }
    next_pid = 1;
    process_total = 0;
}

int process_create(const char *name, uint32_t entry, uint32_t image_start, uint32_t image_end, uint32_t page_directory) {
    if (!page_directory) return -1;
    for (uint32_t i = 0; i < TESTOS_MAX_PROCESSES; ++i) {
        if (processes[i].state != PROCESS_UNUSED) continue;
        processes[i].pid = next_pid++;
        processes[i].parent_pid = 0;
        processes[i].state = PROCESS_READY;
        processes[i].entry = entry;
        /* 0x00800000 is occupied by the embedded initramfs. Keep the user
           stack at 15 MiB so it cannot overwrite the filesystem image. */
        processes[i].user_stack = TESTOS_USER_STACK_TOP;
        processes[i].image_start = image_start;
        processes[i].image_end = image_end;
        processes[i].page_directory = page_directory;
        processes[i].name = name;
        ++process_total;
        return (int)processes[i].pid;
    }
    return -1;
}

const struct process *process_get(uint32_t pid) {
    for (uint32_t i = 0; i < TESTOS_MAX_PROCESSES; ++i)
        if (processes[i].state != PROCESS_UNUSED && processes[i].pid == pid) return &processes[i];
    return 0;
}

uint32_t process_count(void) { return process_total; }
const struct process *process_table(void) { return processes; }
