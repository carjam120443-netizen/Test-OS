#include <stdint.h>
#include "net.h"

/* Minimal Intel E1000/82540EM driver for common QEMU and VirtualBox NICs. */
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC
#define E1000_VENDOR       0x8086
#define E1000_DEVICE       0x100E

#define REG_CTRL   0x0000
#define REG_STATUS 0x0008
#define REG_RAL    0x5400
#define REG_RAH    0x5404
#define REG_IMC    0x00D8
#define REG_TDBAL  0x3800
#define REG_TDLEN  0x3808
#define REG_TDH    0x3810
#define REG_TDT    0x3818
#define REG_RDBAL  0x2800
#define REG_RDLEN  0x2808
#define REG_RDH    0x2810
#define REG_RDT    0x2818
#define REG_RCTL   0x0100
#define REG_TCTL   0x0400
#define REG_TIPG   0x0410

#define CTRL_RST  (1u << 26)
#define STATUS_LU (1u << 1)
#define RCTL_EN   (1u << 1)
#define RCTL_SBP  (1u << 2)
#define RCTL_BAM  (1u << 15)
#define RCTL_SECRC (1u << 26)
#define TCTL_EN   (1u << 1)
#define TCTL_PSP  (1u << 3)
#define TX_CMD_EOP  0x01
#define TX_CMD_IFCS 0x02
#define TX_CMD_RS   0x08
#define DESC_DD      0x01

#define RX_COUNT 8
#define TX_COUNT 8
#define RX_SIZE  2048
#define TX_SIZE  2048

struct e1000_rx_desc {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed));

struct e1000_tx_desc {
    uint64_t addr;
    uint16_t length;
    uint8_t cso;
    uint8_t command;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed));

static volatile uint8_t *mmio;
static struct e1000_rx_desc rx_desc[RX_COUNT] __attribute__((aligned(16)));
static struct e1000_tx_desc tx_desc[TX_COUNT] __attribute__((aligned(16)));
static uint8_t rx_buf[RX_COUNT][RX_SIZE] __attribute__((aligned(16)));
static uint8_t tx_buf[TX_COUNT][TX_SIZE] __attribute__((aligned(16)));
static uint8_t mac[6];
static uint16_t rx_index;
static uint16_t tx_index;
static int ready;
static uint32_t rx_packets;
static uint32_t tx_packets;

static inline void io_outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t io_inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1u << 31) | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFC);
    io_outl(PCI_CONFIG_ADDRESS, address);
    return io_inl(PCI_CONFIG_DATA);
}

static inline void pci_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (1u << 31) | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFC);
    io_outl(PCI_CONFIG_ADDRESS, address);
    io_outl(PCI_CONFIG_DATA, value);
}

static int find_e1000(uint8_t *bus_out, uint8_t *slot_out, uint8_t *func_out) {
    for (uint16_t bus = 0; bus < 256; ++bus) {
        for (uint8_t slot = 0; slot < 32; ++slot) {
            for (uint8_t func = 0; func < 8; ++func) {
                uint32_t id = pci_read((uint8_t)bus, slot, func, 0x00);
                if ((id & 0xFFFFu) == E1000_VENDOR && ((id >> 16) & 0xFFFFu) == E1000_DEVICE) {
                    *bus_out = (uint8_t)bus;
                    *slot_out = slot;
                    *func_out = func;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static inline uint32_t reg_read(uint32_t reg) {
    return *(volatile uint32_t *)(mmio + reg);
}

static inline void reg_write(uint32_t reg, uint32_t value) {
    *(volatile uint32_t *)(mmio + reg) = value;
}

static void delay(void) {
    for (volatile uint32_t i = 0; i < 100000; ++i)
        __asm__ volatile ("nop");
}

static void read_mac(void) {
    uint32_t low = reg_read(REG_RAL);
    uint32_t high = reg_read(REG_RAH);
    mac[0] = low & 0xFF;
    mac[1] = (low >> 8) & 0xFF;
    mac[2] = (low >> 16) & 0xFF;
    mac[3] = (low >> 24) & 0xFF;
    mac[4] = high & 0xFF;
    mac[5] = (high >> 8) & 0xFF;
}

int net_init(void) {
    uint8_t bus, slot, func;
    if (!find_e1000(&bus, &slot, &func)) return 0;

    /* Enable PCI bus mastering and memory-space access. */
    uint32_t command = pci_read(bus, slot, func, 0x04);
    command |= (1u << 1) | (1u << 2);
    pci_write(bus, slot, func, 0x04, command);

    uint32_t bar0 = pci_read(bus, slot, func, 0x10);
    if ((bar0 & 1u) || !(bar0 & 0xFFFFFFF0u)) return 0;
    mmio = (volatile uint8_t *)(uintptr_t)(bar0 & 0xFFFFFFF0u);

    reg_write(REG_IMC, 0xFFFFFFFFu);
    reg_write(REG_CTRL, reg_read(REG_CTRL) | CTRL_RST);
    delay();
    reg_write(REG_IMC, 0xFFFFFFFFu);
    read_mac();

    for (uint16_t i = 0; i < RX_COUNT; ++i) {
        rx_desc[i].addr = (uint64_t)(uintptr_t)rx_buf[i];
        rx_desc[i].status = 0;
    }
    for (uint16_t i = 0; i < TX_COUNT; ++i) {
        tx_desc[i].addr = (uint64_t)(uintptr_t)tx_buf[i];
        tx_desc[i].status = DESC_DD;
    }

    reg_write(REG_RDBAL, (uint32_t)(uintptr_t)rx_desc);
    reg_write(REG_RDBAL + 4, 0);
    reg_write(REG_RDLEN, sizeof(rx_desc));
    reg_write(REG_RDH, 0);
    reg_write(REG_RDT, RX_COUNT - 1);
    reg_write(REG_RCTL, RCTL_EN | RCTL_BAM | RCTL_SECRC);

    reg_write(REG_TDBAL, (uint32_t)(uintptr_t)tx_desc);
    reg_write(REG_TDBAL + 4, 0);
    reg_write(REG_TDLEN, sizeof(tx_desc));
    reg_write(REG_TDH, 0);
    reg_write(REG_TDT, 0);
    reg_write(REG_TCTL, TCTL_EN | TCTL_PSP | (0x10u << 4) | (0x40u << 12));
    reg_write(REG_TIPG, 0x0060200Au);

    rx_index = 0;
    tx_index = 0;
    ready = 1;
    return 1;
}

void net_poll(void) {
    if (!ready) return;

    struct e1000_rx_desc *d = &rx_desc[rx_index];
    if (d->status & DESC_DD) {
        ++rx_packets;
        d->status = 0;
        reg_write(REG_RDT, rx_index);
        rx_index = (rx_index + 1) % RX_COUNT;
    }
}

int net_is_ready(void) { return ready; }
int net_link_up(void) { return ready && (reg_read(REG_STATUS) & STATUS_LU); }
uint32_t net_rx_packets(void) { return rx_packets; }
uint32_t net_tx_packets(void) { return tx_packets; }

void net_get_mac(uint8_t out[6]) {
    for (int i = 0; i < 6; ++i) out[i] = mac[i];
}
