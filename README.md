# Test-OS 🖥️

**Test-OS** is a custom x86 operating-system project built from the ground up for learning, experimentation, and absolutely unnecessary levels of customization. 😈

## Current status

🚧 **Version 0.1.0 — Kernel foundation**

The project currently contains:

- 🥾 Multiboot-compatible boot entry
- 🧠 Initial freestanding C kernel
- 🖥️ VGA text terminal output
- 🔗 Kernel linker script
- ⚙️ Make-based build system
- 💿 GRUB ISO generation
- 🤖 GitHub Actions automated ISO builds
- 📜 Custom Test-OS Community License

## Roadmap

- [ ] Interactive shell
- [ ] Keyboard driver
- [ ] Interrupt handling
- [ ] Timer support
- [ ] Physical and virtual memory management
- [ ] Heap allocator
- [ ] Filesystem support
- [ ] Disk drivers
- [ ] Userspace and program loading
- [ ] Networking
- [ ] `pkg` package manager
- [ ] System utilities
- [ ] GUI/window system
- [ ] More architectures

## Building

The project is currently targeting x86 and is designed to be built with NASM, a freestanding C compiler, GNU binutils, GRUB tooling, and xorriso.

```bash
make
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
