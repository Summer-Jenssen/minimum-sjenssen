#include "minemu/platform.h"

void uart_print(char* str, int len){
    int total_bytes_printed = 0;
    while (total_bytes_printed < len){
    // first check if TX is ready
    if (MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY) {
    // send a single byte
    MINEMU_UART0->tx_data = (uint32_t)str[total_bytes_printed];
    total_bytes_printed = total_bytes_printed + 1;
    }
    }
}