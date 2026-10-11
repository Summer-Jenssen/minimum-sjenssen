#include "minemu/platform.h"
#include "minemu/trap.h"
#include "minemu/irq.h"
#include "minemu/user_abi.h"
#include <stdint.h>

struct uart_rx_buffer{
    uint32_t buffer[MINEMU_UART_RX_CAPACITY]; //"global" buffer to hold everything read from the uart!
    uint32_t nbytes_available; //keeps track of nbytes free. = MINEMU_UART_RX_CAPACITY-nbytes_used, starting from index 0 to nbytes_used
    uint32_t next_read; //tracks what the next byte (index) to read out of the buffer is
    uint32_t next_write; //tracks the next byte (index) to be written in
};

static struct uart_rx_buffer uart0_rx_buffer; 
static struct uart_rx_buffer uart1_rx_buffer; 

static int uart0_fd = MINEMU_FD_UART_OUT;
static int uart1_fd = MINEMU_FD_UART_IN;

void uart0_init(){
    uart0_rx_buffer.nbytes_available = MINEMU_UART_RX_CAPACITY;
    uart0_rx_buffer.next_read = 0;
    uart0_rx_buffer.next_write = 0;
}

void uart1_init(){
    uart1_rx_buffer.nbytes_available = MINEMU_UART_RX_CAPACITY;
    uart1_rx_buffer.next_read = 0;
    uart1_rx_buffer.next_write = 0;
}

void uart_init(){
    uart0_init();
    uart1_init();
}

void clear_uart0_rx_buffer(){
    minemu_irq_disable(); //disable here bc we don't know if it would already be when called. 
    uart0_rx_buffer.nbytes_available = MINEMU_UART_RX_CAPACITY;
    uart0_rx_buffer.next_read = 0;
    uart0_rx_buffer.next_write = 0;
    //maybe zero out the data? Not really important, guess 
    //note: stdin buffer gets cleared when \n is read (?), so yes, zero it out bc may want to implement
    for (uint32_t i = 0; i < MINEMU_UART_RX_CAPACITY; i++){
        uart0_rx_buffer.buffer[i] = '\0'; //0 it out!
    }
    minemu_irq_enable();
}

void clear_uart1_rx_buffer(){
    minemu_irq_disable(); //disable here bc we don't know if it would already be when called. 
    uart1_rx_buffer.nbytes_available = MINEMU_UART_RX_CAPACITY;
    uart1_rx_buffer.next_read = 0;
    uart1_rx_buffer.next_write = 0;
    //maybe zero out the data? Not really important, guess 
    //note: stdin buffer gets cleared when \n is read (?), so yes, zero it out bc may want to implement
    for (uint32_t i = 0; i < MINEMU_UART_RX_CAPACITY; i++){
        uart1_rx_buffer.buffer[i] = '\0'; //0 it out!
    }
    minemu_irq_enable();
}

void clear_uart_buffer(int fd){
    if(fd == uart0_fd){
        clear_uart0_rx_buffer();
    } else if (fd == uart1_fd){
        clear_uart1_rx_buffer();
    }
}

int writeb_uart0_rx_buffer(uint32_t byte){
    //don't need minemu_irq_disable() here bc we can gauantee it'll only be used during an interrupt = already diabled
    if (uart0_rx_buffer.nbytes_available > 0){
        uart0_rx_buffer.buffer[uart0_rx_buffer.next_write] = byte;
        uart0_rx_buffer.nbytes_available--;
        uart0_rx_buffer.next_write = (uart0_rx_buffer.next_write + 1) % MINEMU_UART_RX_CAPACITY; 
        //modding it allows the buffer to wrap around and reuse bytes that were already read!
        return 0;
    }
    //else, don't write bc its full :(
    return 1; //failure :(
}

int writeb_uart1_rx_buffer(uint32_t byte){
    //don't need minemu_irq_disable() here bc we can gauantee it'll only be used during an interrupt = already diabled
    if (uart1_rx_buffer.nbytes_available > 0){
        uart1_rx_buffer.buffer[uart1_rx_buffer.next_write] = byte;
        uart1_rx_buffer.nbytes_available--;
        uart1_rx_buffer.next_write = (uart1_rx_buffer.next_write + 1) % MINEMU_UART_RX_CAPACITY; 
        //modding it allows the buffer to wrap around and reuse bytes that were already read!
        return 0;
    }
    //else, don't write bc its full :(
    return 1; //failure :(
}

int writeb_rx_buffer(uint32_t byte, int fd){
    if(fd == uart0_fd){
        return writeb_uart0_rx_buffer(byte);
    } else if(fd == uart1_fd){
       return writeb_uart1_rx_buffer(byte);
    }

    return 1;
}

//Accepts a pointer to a byte. Writes what was read to it before returning a success(0)/failure(1) code. fails if nothing to read in buffer
int readb_uart0_rx_buffer(uint32_t* byte){
    minemu_irq_disable();
    if(uart0_rx_buffer.nbytes_available < MINEMU_UART_RX_CAPACITY){ //checks if there are bytes to read! If all bytes are available, do nothing!
        *byte = uart0_rx_buffer.buffer[uart0_rx_buffer.next_read];
        uart0_rx_buffer.next_read = (uart0_rx_buffer.next_read + 1) % MINEMU_UART_RX_CAPACITY; //lets it wrap around :D 
        uart0_rx_buffer.nbytes_available++;
        minemu_irq_enable();
        return 0;
    }
    minemu_irq_enable();
    return 1;
}

//Accepts a pointer to a byte. Writes what was read to it before returning a success(0)/failure(1) code. fails if nothing to read in buffer
int readb_uart1_rx_buffer(uint32_t* byte){
    minemu_irq_disable();
    if(uart1_rx_buffer.nbytes_available < MINEMU_UART_RX_CAPACITY){ //checks if there are bytes to read! If all bytes are available, do nothing!
        *byte = uart1_rx_buffer.buffer[uart1_rx_buffer.next_read];
        uart1_rx_buffer.next_read = (uart1_rx_buffer.next_read + 1) % MINEMU_UART_RX_CAPACITY; //lets it wrap around :D 
        uart1_rx_buffer.nbytes_available++;
        minemu_irq_enable();
        return 0;
    }
    minemu_irq_enable();
    return 1;
}

int readb_uart_rx_buffer(uint32_t* byte, int fd){
    if(fd == uart0_fd){
        return readb_uart0_rx_buffer(byte);
    } else if (fd == uart1_fd){
        return readb_uart1_rx_buffer(byte);
    }
    return 1;
}

int uart0_print(char* str, int len){
    int total_bytes_printed = 0;
    while (total_bytes_printed < len){
    // first check if TX is ready
    if (MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY) {
    // send a single byte
    MINEMU_UART0->tx_data = (uint32_t)str[total_bytes_printed];
    total_bytes_printed = total_bytes_printed + 1;
        }
    }
    return total_bytes_printed;
}

int uart1_print(char* str, int len){
    int total_bytes_printed = 0;
    while (total_bytes_printed < len){
    // first check if TX is ready
    if (MINEMU_UART1->status & MINEMU_UART_STATUS_TX_READY) {
    // send a single byte
    MINEMU_UART1->tx_data = (uint32_t)str[total_bytes_printed];
    total_bytes_printed = total_bytes_printed + 1;
        }
    }
    return total_bytes_printed;
}

int uart_print(char* str, int len, int fd){
    if(fd == uart0_fd){
        return uart0_print(str, len);
    } else if (fd == uart1_fd){
        return uart1_print(str, len);

    }
    return 1;
}

void uart0_handler(struct minemu_trap_frame *frame){
    (void)frame; //not using frame so we can ignore it :)
    while(MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) {
        uint32_t byte = MINEMU_UART0->rx_data; // read a single byte
        writeb_uart0_rx_buffer(byte);
    }
}

void uart1_handler(struct minemu_trap_frame *frame){
    (void)frame; //not using frame so we can ignore it :)
    while(MINEMU_UART1->status & MINEMU_UART_STATUS_RX_READY) {
        uint32_t byte = MINEMU_UART1->rx_data; // read a single byte
        writeb_uart1_rx_buffer(byte);
    }
}