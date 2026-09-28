#ifndef MINEMU_UART_H
#define MINEMU_UART_H

void uart_print(char* str, int len);
void uart_handler(struct minemu_trap_frame *frame);

#endif