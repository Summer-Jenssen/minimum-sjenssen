#ifndef MINEMU_UART_H
#define MINEMU_UART_H
#include "minemu/trap.h"

int uart_print(char* str, int len, int fd);
void uart0_handler(struct minemu_trap_frame *frame);
void uart1_handler(struct minemu_trap_frame *frame);
void uart_init();
void clear_uart_buffer(int fd);
int readb_uart_rx_buffer(uint32_t* byte, int fd);
int writeb_rx_buffer(uint32_t byte, int fd);

#endif