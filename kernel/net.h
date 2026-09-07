#ifndef TEST_OS_NET_H
#define TEST_OS_NET_H

#include <stdint.h>

/* Minimal network-driver interface used by the kernel shell. */
int net_init(void);
void net_poll(void);
int net_is_ready(void);
int net_link_up(void);
uint32_t net_rx_packets(void);
uint32_t net_tx_packets(void);
void net_get_mac(uint8_t out[6]);

#endif
