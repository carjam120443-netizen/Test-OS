# XFCE Port

Test-OS is beginning an XFCE-compatible desktop stack.

## Current state

The real XFCE desktop cannot run directly yet because Test-OS does not have the Unix/POSIX environment that XFCE and GTK expect. This directory contains the Test-OS-side integration layer that will eventually host XFCE.

## Port plan

1. Real ring-3 processes
2. Virtual memory and paging
3. `int 0x80` syscall ABI
4. ELF executable loader
5. VFS and filesystem support
6. Initramfs and shared libraries
7. Framebuffer graphics
8. Keyboard and mouse input
9. Test-OS libc/POSIX compatibility layer
10. Wayland/X11-compatible display protocol layer
11. GLib/GTK compatibility
12. XFCE session and core components

## Target desktop

The eventual boot flow is intended to become:

```text
bootloader -> kernel -> init -> display server -> xfce4-session -> XFCE desktop
```

Until the lower layers exist, the current kernel shell remains the primary interface.
