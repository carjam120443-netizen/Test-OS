AS=nasm
CC=gcc
LD=ld
ASFLAGS=-f elf32
CFLAGS=-m32 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -Iinclude
USER_CFLAGS=-m32 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -nostdlib -Iinclude
LDFLAGS=-T kernel/linker.ld -m elf_i386

BUILD=build

all: $(BUILD)/testos.bin

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: boot/boot.asm | $(BUILD)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/kernel.o: kernel/kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/net.o: kernel/net.c include/net.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/process.o: kernel/process.c kernel/process.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/syscall.o: kernel/syscall.c kernel/syscall.h kernel/process.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/testos.bin: $(BUILD)/boot.o $(BUILD)/kernel.o $(BUILD)/net.o $(BUILD)/process.o $(BUILD)/syscall.o kernel/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(BUILD)/boot.o $(BUILD)/kernel.o $(BUILD)/net.o $(BUILD)/process.o $(BUILD)/syscall.o

$(BUILD)/init.o: user/init.c include/syscall.h | $(BUILD)
	$(CC) $(USER_CFLAGS) -c $< -o $@

userspace: $(BUILD)/init.o
	@echo "Userspace init compiled: $(BUILD)/init.o"

iso: $(BUILD)/testos.bin
	mkdir -p $(BUILD)/iso/boot/grub
	cp $(BUILD)/testos.bin $(BUILD)/iso/boot/testos.bin
	cp grub/grub.cfg $(BUILD)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD)/testos.iso $(BUILD)/iso

clean:
	rm -rf $(BUILD)

.PHONY: all userspace iso clean
