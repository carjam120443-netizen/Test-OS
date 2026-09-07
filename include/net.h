#ifndef TESTOS_NET_H
#define TESTOS_NET_H

#include <stdint.h>

int net_init(void);
void net_poll(void);
int net_is_ready(void);
int net_link_up(void);
uint32_t net_rx_packets(void);
uint32_t net_tx_packets(void);
void net_get_mac(uint8_t mac[6]);

#endif
