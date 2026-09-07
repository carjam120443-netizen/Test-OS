#include "libc.h"

void user_init(void) {
    puts("Test-OS userspace: init is running in ring 3.");
    puts("libc: strlen/memcpy/memset/strcmp/puts are available.");
    puts("filesystem: /bin/init was loaded from initramfs as ELF32.");
    for (;;) sys_yield();
}
