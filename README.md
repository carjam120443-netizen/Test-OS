# Test-OS 🖥️

> [!WARNING]
> **Test-OS is basically retired / inactive.** This was an old operating-system experiment built for learning and experimentation. I don't currently plan to actively develop or maintain it, so don't expect regular updates, fixes, or new releases. The repository is being kept up as a record of the project and its experiments.

## About

**Test-OS** is a custom x86 operating-system project built from the ground up for learning, experimentation, and absolutely unnecessary levels of customization. 😈

The project explored ideas such as a freestanding C kernel, a text terminal, keyboard input, early networking-driver work, a process table, and a basic userspace syscall interface. Its code, build instructions, roadmap, and feature list below describe the project as it existed during development; they may be incomplete or outdated.

## Historical project status

The project reached an early kernel and userspace foundation, including:

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

These features were experimental foundations, not a claim that Test-OS became a complete or production-ready operating system.

## Historical userspace architecture

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

## Old roadmap

The checklist below is preserved for historical reference only; unchecked items are **not promises of future development**.

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

## Historical build instructions

These instructions are kept in case you want to experiment with the source, but they may no longer work with current toolchains or dependencies. The project targeted x86 and used NASM, GCC, GNU binutils, GRUB tooling, and xorriso.

```bash
make
make userspace
make iso
```

The ISO was configured to be placed at `build/testos.iso`.

## Historical emulator instructions

Test-OS was intended to be tested in an emulator such as QEMU before any real-hardware experimentation.

```bash
qemu-system-i386 -cdrom build/testos.iso
```

## License

Test-OS uses the **Test-OS Community License**, included in [`LICENSE`](LICENSE).

Fork it, study it, or use it as a snapshot of an old experiment. 🚀