#ifndef TEST_OS_SYSCALL_H
#define TEST_OS_SYSCALL_H

#include <stdint.h>

#define SYS_EXIT  1
#define SYS_WRITE 2
#define SYS_YIELD 3
#define SYS_GETPID 4

uint32_t syscall_dispatch(uint32_t number, uint32_t arg0, uint32_t arg1, uint32_t arg2);

#endif
