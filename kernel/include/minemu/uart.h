#ifndef MINEMU_UART_H
#define MINEMU_UART_H
#include "minemu/trap.h"

void uart_print(char* str, int len);
void uart_handler(struct minemu_trap_frame *frame);
void uart_init();
void clear_uart_rx_buffer();
int readb_uart_rx_buffer(uint32_t* byte);

#endif