#include "minemu/platform.h"
#include "minemu/trap.h"
#include "minemu/irq.h"
#include <stdint.h>

struct uart_rx_buffer{
    uint32_t buffer[MINEMU_UART_RX_CAPACITY]; //"global" buffer to hold everything read from the uart!
    uint32_t nbytes_available; //keeps track of nbytes free. = MINEMU_UART_RX_CAPACITY-nbytes_used, starting from index 0 to nbytes_used
    uint32_t next_read; //tracks what the next byte (index) to read out of the buffer is
    uint32_t next_write; //tracks the next byte (index) to be written in
    //need to figure out when to clear the buffer... maybe add func to free it? 
    //look at when stdin is cleared for reference!
};

static struct uart_rx_buffer rx_buffer; 
// static int counter = 0;

void uart_init(){
    rx_buffer.nbytes_available = MINEMU_UART_RX_CAPACITY;
    rx_buffer.next_read = 0;
    rx_buffer.next_write = 0;
}

void clear_uart_rx_buffer(){
    minemu_irq_disable(); //disable here bc we don't know if it would already be when called. 
    rx_buffer.nbytes_available = MINEMU_UART_RX_CAPACITY;
    rx_buffer.next_read = 0;
    rx_buffer.next_write = 0;
    //maybe zero out the data? Not really important, guess 
    //note: stdin buffer gets cleared when \n is read (?), so yes, zero it out bc may want to implement
    for (uint32_t i = 0; i < MINEMU_UART_RX_CAPACITY; i++){
        rx_buffer.buffer[i] = '\0'; //0 it out!
    }
    minemu_irq_enable();
}

int writeb_uart_rx_buffer(uint32_t byte){
    //don't need minemu_irq_disable() here bc we can gauantee it'll only be used during an interrupt = already diabled
    if (rx_buffer.nbytes_available > 0){
        rx_buffer.buffer[rx_buffer.next_write] = byte;
        rx_buffer.nbytes_available--;
        rx_buffer.next_write = (rx_buffer.next_write + 1) % MINEMU_UART_RX_CAPACITY; 
        //modding it allows the buffer to wrap around and reuse bytes that were already read!
        return 0;
    }
    //else, don't write bc its full :(
    return 1; //failure :(
}

//Accepts a pointer to a byte. Writes what was read to it before returning a success(0)/failure(1) code. fails if nothing to read in buffer
int readb_uart_rx_buffer(uint32_t* byte){
    minemu_irq_disable();
    if(rx_buffer.nbytes_available < MINEMU_UART_RX_CAPACITY){ //checks if there are bytes to read! If all bytes are available, do nothing!
        *byte = rx_buffer.buffer[rx_buffer.next_read];
        rx_buffer.next_read = (rx_buffer.next_read + 1) % MINEMU_UART_RX_CAPACITY; //lets it wrap around :D 
        rx_buffer.nbytes_available++;
        minemu_irq_enable();
        return 0;
    }
    minemu_irq_enable();
    return 1;
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

void uart_handler(struct minemu_trap_frame *frame){
    (void)frame; //not using frame so we can ignore it :)
    // counter++;
    // char digit = (char)('0' + counter);
    // uart_print(&digit, 1);
    while(MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) {
        uint32_t byte = MINEMU_UART0->rx_data; // read a single byte
        writeb_uart_rx_buffer(byte);
    }
}