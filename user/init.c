#include "syscall.h"

void _start(void) {
    sys_write("Test-OS userspace: init ABI online.\n");
    sys_write("The next stage is ring-3 execution and program loading.\n");

    for (;;) sys_yield();
}
