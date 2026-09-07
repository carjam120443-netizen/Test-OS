#ifndef TEST_OS_USER_SYSCALL_H
#define TEST_OS_USER_SYSCALL_H

#define SYS_EXIT  1
#define SYS_WRITE 2
#define SYS_YIELD 3
#define SYS_GETPID 4

static inline int sys_write(const char *text) {
    int result;
    __asm__ volatile ("int $0x80" : "=a"(result) : "a"(SYS_WRITE), "b"(text) : "memory");
    return result;
}

static inline int sys_getpid(void) {
    int result;
    __asm__ volatile ("int $0x80" : "=a"(result) : "a"(SYS_GETPID) : "memory");
    return result;
}

static inline void sys_yield(void) {
    __asm__ volatile ("int $0x80" : : "a"(SYS_YIELD) : "memory");
}

static inline void sys_exit(int status) {
    __asm__ volatile ("int $0x80" : : "a"(SYS_EXIT), "b"(status) : "memory");
    for (;;) __asm__ volatile ("hlt");
}

#endif
