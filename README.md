# Test-OS 🖥️

**Test-OS** is a custom x86 operating-system project built from the ground up for learning, experimentation, and absolutely unnecessary levels of customization. 😈

## Current status

🚧 **Version 0.2.0 — Kernel + networking + userspace foundation**

The project currently contains:

- 🥾 Multiboot-compatible boot entry
- 🧠 Freestanding x86 C kernel
- 🖥️ VGA text terminal
- ⌨️ PS/2 keyboard polling
- 🐚 Interactive kernel shell
- 🌐 Intel E1000/82540EM network-driver foundation
- 🧩 Kernel process table with PID allocation
- 📞 Userspace syscall ABI (`write`, `getpid`, `yield`, `exit`)
- 🚀 First userspace `init` program source
- 🔗 Kernel linker script
- ⚙️ Make-based build system
- 💿 GRUB ISO generation
- 🤖 GitHub Actions automated ISO builds
- 📜 Custom Test-OS Community License

## Userspace architecture

Test-OS is now being split into a privileged kernel and a future userspace environment:

```text
Test-OS
├── Kernel
│   ├── Process manager
│   ├── Syscall dispatcher
│   ├── Network driver
│   └── Hardware drivers
│
└── Userspace
    └── init
```

The current userspace code is compiled separately from the kernel. Ring-3 execution, paging/address spaces, ELF loading, and a real scheduler are the next stages.

## Roadmap

- [x] Interactive shell
- [x] Keyboard driver foundation
- [x] Networking driver foundation
- [x] Process table
- [x] Userspace syscall ABI
- [x] Userspace init source
- [ ] GDT/IDT and interrupt handling
- [ ] Timer and preemptive scheduling
- [ ] Physical memory manager
- [ ] Paging and per-process address spaces
- [ ] Ring-3 userspace execution
- [ ] ELF program loader
- [ ] Heap allocator
- [ ] VFS and filesystem support
- [ ] Disk drivers
- [ ] Full Ethernet/ARP/IPv4 stack
- [ ] DHCP, DNS, UDP and TCP
- [ ] `pkg` package manager
- [ ] Test-OS libc
- [ ] Framebuffer graphics
- [ ] Mouse/input subsystem
- [ ] Window/display system
- [ ] GTK/GLib compatibility layer
- [ ] XFCE port
- [ ] More architectures

## Building

The project currently targets x86 and is designed to be built with NASM, GCC, GNU binutils, GRUB tooling, and xorriso.

```bash
make
make userspace
make iso
```

The resulting ISO will be placed at `build/testos.iso`.

## Running

Test-OS is intended to be tested in an emulator such as QEMU before being considered for real hardware.

```bash
qemu-system-i386 -cdrom build/testos.iso
```

## License

Test-OS uses the **Test-OS Community License**, included in [`LICENSE`](LICENSE).

Fork it. Break it. Fix it. Turn it into something completely ridiculous. 🚀
