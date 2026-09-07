#include <stdint.h>
#include "net.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t*)0xB8000)
#define KEYBOARD_DATA 0x60
#define KEYBOARD_STATUS 0x64

static uint8_t row = 0;
static uint8_t column = 0;
static uint8_t color = 0x07;
static char input[80];
static uint8_t input_length = 0;

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void terminal_clear(void) {
    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i)
        VGA_MEMORY[i] = ((uint16_t)color << 8) | ' ';
    row = 0;
    column = 0;
}

static void terminal_scroll(void) {
    if (row < VGA_HEIGHT) return;
    for (uint32_t y = 1; y < VGA_HEIGHT; ++y)
        for (uint32_t x = 0; x < VGA_WIDTH; ++x)
            VGA_MEMORY[(y - 1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
    for (uint32_t x = 0; x < VGA_WIDTH; ++x)
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ((uint16_t)color << 8) | ' ';
    row = VGA_HEIGHT - 1;
}

static void terminal_putchar(char c) {
    if (c == '\n') {
        column = 0;
        ++row;
        terminal_scroll();
        return;
    }
    if (c == '\b') {
        if (column > 0) {
            --column;
            VGA_MEMORY[row * VGA_WIDTH + column] = ((uint16_t)color << 8) | ' ';
        }
        return;
    }
    VGA_MEMORY[row * VGA_WIDTH + column] = ((uint16_t)color << 8) | (uint8_t)c;
    if (++column >= VGA_WIDTH) {
        column = 0;
        ++row;
        terminal_scroll();
    }
}

static void terminal_write(const char* text) {
    while (*text) terminal_putchar(*text++);
}

static void terminal_write_hex8(uint8_t value) {
    static const char hex[] = "0123456789ABCDEF";
    terminal_putchar(hex[(value >> 4) & 0xF]);
    terminal_putchar(hex[value & 0xF]);
}

static void terminal_write_dec(uint32_t value) {
    char buffer[11];
    int i = 0;
    if (value == 0) { terminal_putchar('0'); return; }
    while (value && i < (int)sizeof(buffer)) {
        buffer[i++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (i--) terminal_putchar(buffer[i]);
}

static int string_equal(const char* a, const char* b) {
    while (*a && *b && *a == *b) { ++a; ++b; }
    return *a == *b;
}

static void print_network_status(void) {
    uint8_t mac[6];
    net_get_mac(mac);
    terminal_write("Network driver: ");
    terminal_write(net_is_ready() ? "E1000 READY\n" : "NO NIC FOUND\n");
    if (!net_is_ready()) return;
    terminal_write("Link: ");
    terminal_write(net_link_up() ? "UP\n" : "DOWN\n");
    terminal_write("MAC: ");
    for (int i = 0; i < 6; ++i) {
        terminal_write_hex8(mac[i]);
        if (i != 5) terminal_putchar(':');
    }
    terminal_putchar('\n');
    terminal_write("RX packets: ");
    terminal_write_dec(net_rx_packets());
    terminal_putchar('\n');
    terminal_write("TX packets: ");
    terminal_write_dec(net_tx_packets());
    terminal_putchar('\n');
}

static void shell_prompt(void) {
    terminal_write("testos> ");
    input_length = 0;
}

static void shell_command(void) {
    input[input_length] = '\0';

    if (input_length == 0) {
        shell_prompt();
    } else if (string_equal(input, "help")) {
        terminal_write("Commands:\n");
        terminal_write("  help   - show this help\n");
        terminal_write("  about  - show system information\n");
        terminal_write("  clear  - clear the terminal\n");
        terminal_write("  echo   - print text\n");
        terminal_write("  net    - show network adapter status\n");
        terminal_write("  reboot - reboot the machine\n\n");
        shell_prompt();
    } else if (string_equal(input, "about")) {
        terminal_write("Test-OS 0.1.0\n");
        terminal_write("A tiny experimental x86 operating system.\n");
        terminal_write("Kernel: C + x86 assembly\n\n");
        shell_prompt();
    } else if (string_equal(input, "net")) {
        print_network_status();
        terminal_putchar('\n');
        shell_prompt();
    } else if (string_equal(input, "clear")) {
        terminal_clear();
        shell_prompt();
    } else if (input_length >= 5 && input[0] == 'e' && input[1] == 'c' && input[2] == 'h' && input[3] == 'o' && input[4] == ' ') {
        terminal_write(input + 5);
        terminal_putchar('\n');
        shell_prompt();
    } else if (string_equal(input, "reboot")) {
        terminal_write("Rebooting...\n");
        uint8_t good = 0x02;
        while (good & 0x02) good = inb(0x64);
        outb(0x64, 0xFE);
        for (;;) __asm__ volatile ("hlt");
    } else {
        terminal_write("Unknown command. Type 'help'.\n");
        shell_prompt();
    }
}

static char keyboard_translate(uint8_t scancode) {
    static const char map[128] = {
        0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b', '\t',
        'q','w','e','r','t','y','u','i','o','p','[',']','\n', 0, 'a','s',
        'd','f','g','h','j','k','l',';','\'', '`', 0, '\\','z','x','c','v',
        'b','n','m',',','.','/', 0, '*', 0, ' ', 0
    };
    if (scancode & 0x80) return 0;
    return scancode < 128 ? map[scancode] : 0;
}

static void keyboard_poll(void) {
    if (!(inb(KEYBOARD_STATUS) & 1)) return;
    uint8_t scancode = inb(KEYBOARD_DATA);
    char c = keyboard_translate(scancode);
    if (!c) return;

    if (c == '\n') {
        terminal_putchar('\n');
        shell_command();
    } else if (c == '\b') {
        if (input_length > 0) {
            --input_length;
            terminal_putchar('\b');
        }
    } else if (c >= 32 && c <= 126 && input_length < sizeof(input) - 1) {
        input[input_length++] = c;
        terminal_putchar(c);
    }
}

void kernel_main(void) {
    terminal_clear();
    terminal_write("===========================\n");
    terminal_write("       TEST-OS KERNEL      \n");
    terminal_write("===========================\n\n");
    terminal_write("Test-OS has booted successfully!\n");
    terminal_write("Kernel: 0.1.0\n");
    terminal_write("Architecture: x86\n");
    terminal_write("Status: ONLINE\n");
    terminal_write("Initializing network...\n");
    terminal_write(net_init() ? "Network: E1000 initialized\n\n" : "Network: no supported E1000 NIC\n\n");
    terminal_write("Type 'help' for available commands.\n\n");
    shell_prompt();

    for (;;) {
        keyboard_poll();
        net_poll();
        __asm__ volatile ("hlt");
    }
}
