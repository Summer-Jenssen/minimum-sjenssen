#include "minemu/platform.h"
#include "minemu/trap.h"

struct uart_rx_buffer{
    uint32_t buffer[MINEMU_UART_RX_CAPACITY]; //"global" buffer to hold everything read from the uart!
    int nbytes_used;
    int nbytes_available; //keeps track of nbytes free. = MINEMU_UART_RX_CAPACITY-nbytes_used, starting from index 0 to nbytes_used
    int next_read; //tracks what the next byte (index) to read out of the buffer is
    int next_write; //tracks the next byte (index) to be written in
    //need to figure out when to clear the buffer... maybe add func to free it? 
};

static uart_rx_buffer; 

void uart_init(){
    uart_rx_buffer.nbytes_available = MINEMU_UART_RX_CAPACITY;
    uart_rx_buffer.next_read = 0;
    uart_rx_buffer.next_write = 0;
    //idk what else to put here, maybe useful later
}

void clear_uart_rx_buffer(){
    uart_rx_buffer.nbytes_available = MINEMU_UART_RX_CAPACITY;
    uart_rx_buffer.next_read = 0;
    uart_rx_buffer.next_write = 0;
    //maybe zero out the data? Not really important, guess 
}

void uart_handler(struct minemu_trap_frame *frame){
    (void)frame; //not using frame so we can ignore it :)

    while(MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) {
    uint32_t byte = MINEMU_UART0->rx_data; // read a single byte

    }
}

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