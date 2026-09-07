#include "syscall.h"
#include "process.h"

/*
 * Syscall dispatch is deliberately kept separate from the eventual x86
 * interrupt/trap entry point. Once ring 3 is enabled, int 0x80 will feed
 * its register arguments into this function.
 */
uint32_t syscall_dispatch(uint32_t number, uint32_t arg0, uint32_t arg1, uint32_t arg2) {
    (void)arg1;
    (void)arg2;

    switch (number) {
        case SYS_EXIT:
            return 0;
        case SYS_WRITE:
            /* Console/file descriptors will be implemented by the VFS layer. */
            return arg0;
        case SYS_YIELD:
            return 0;
        case SYS_GETPID:
            return process_count() ? process_table()[0].pid : 0;
        default:
            return (uint32_t)-1;
    }
}
