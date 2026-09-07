#ifndef TEST_OS_PAGING_H
#define TEST_OS_PAGING_H

#include <stdint.h>

int paging_init(void);
uint32_t paging_create_user_space(void);
int paging_switch(uint32_t page_directory);
uint32_t paging_current(void);

#endif
