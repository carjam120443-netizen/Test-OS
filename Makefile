AS=nasm
CC=gcc
LD=ld
OBJCOPY=objcopy
PYTHON=python3
ASFLAGS=-f elf32
CFLAGS=-m32 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -fno-pie -fno-pic -fno-builtin -Iinclude -Ikernel -Idesktop
USER_CFLAGS=-m32 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -fno-pie -fno-pic -fno-builtin -nostdlib -Iinclude -Iuser
LDFLAGS=-T kernel/linker.ld -m elf_i386 -z noexecstack
BUILD=build
all: $(BUILD)/testos.bin
$(BUILD):
	mkdir -p $(BUILD)
$(BUILD)/boot.o: boot/boot.asm | $(BUILD)
	$(AS) $(ASFLAGS) $< -o $@
$(BUILD)/syscall_isr.o: kernel/syscall_isr.asm | $(BUILD)
	$(AS) $(ASFLAGS) $< -o $@
$(BUILD)/kernel.o: kernel/kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/net.o: kernel/net.c kernel/net.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/process.o: kernel/process.c kernel/process.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/syscall.o: kernel/syscall.c kernel/syscall.h kernel/process.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/gdt.o: kernel/gdt.c kernel/gdt.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/tss.o: kernel/tss.c kernel/tss.h kernel/gdt.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/paging.o: kernel/paging.c kernel/paging.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/idt.o: kernel/idt.c kernel/idt.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/fs.o: kernel/fs.c kernel/fs.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/elf.o: kernel/elf.c kernel/elf.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/graphics.o: kernel/graphics.c kernel/graphics.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/compositor.o: kernel/compositor.c kernel/compositor.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/desktop.o: desktop/desktop.c desktop/desktop.h kernel/graphics.h kernel/compositor.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/xfce_session.o: desktop/xfce/xfce_session.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/libc.o: user/libc.c user/libc.h include/syscall.h | $(BUILD)
	$(CC) $(USER_CFLAGS) -c $< -o $@
$(BUILD)/init.o: user/init.c user/libc.h include/syscall.h | $(BUILD)
	$(CC) $(USER_CFLAGS) -c $< -o $@
$(BUILD)/init.elf: $(BUILD)/libc.o $(BUILD)/init.o user/user.ld
	$(LD) -m elf_i386 -T user/user.ld -o $@ $(BUILD)/libc.o $(BUILD)/init.o
$(BUILD)/initramfs.img: $(BUILD)/init.elf tools/mkinitramfs.py
	$(PYTHON) tools/mkinitramfs.py $(BUILD)/init.elf $@
$(BUILD)/initramfs.o: $(BUILD)/initramfs.img
	$(OBJCOPY) -I binary -O elf32-i386 -B i386 $< $@
	$(OBJCOPY) --rename-section .data=.initramfs,alloc,load,readonly,data,contents $@
$(BUILD)/testos.bin: $(BUILD)/boot.o $(BUILD)/syscall_isr.o $(BUILD)/kernel.o $(BUILD)/net.o $(BUILD)/process.o $(BUILD)/syscall.o $(BUILD)/gdt.o $(BUILD)/tss.o $(BUILD)/paging.o $(BUILD)/idt.o $(BUILD)/fs.o $(BUILD)/elf.o $(BUILD)/graphics.o $(BUILD)/compositor.o $(BUILD)/desktop.o $(BUILD)/xfce_session.o $(BUILD)/initramfs.o kernel/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(BUILD)/boot.o $(BUILD)/syscall_isr.o $(BUILD)/kernel.o $(BUILD)/net.o $(BUILD)/process.o $(BUILD)/syscall.o $(BUILD)/gdt.o $(BUILD)/tss.o $(BUILD)/paging.o $(BUILD)/idt.o $(BUILD)/fs.o $(BUILD)/elf.o $(BUILD)/graphics.o $(BUILD)/compositor.o $(BUILD)/desktop.o $(BUILD)/xfce_session.o $(BUILD)/initramfs.o
userspace: $(BUILD)/init.elf
	@echo "Userspace ELF: $(BUILD)/init.elf"
iso: $(BUILD)/testos.bin
	mkdir -p $(BUILD)/iso/boot/grub
	cp $(BUILD)/testos.bin $(BUILD)/iso/boot/testos.bin
	cp grub/grub.cfg $(BUILD)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD)/testos.iso $(BUILD)/iso
clean:
	rm -rf $(BUILD)
.PHONY: all userspace iso clean
