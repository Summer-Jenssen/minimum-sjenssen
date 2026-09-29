#include "minemu/uart.h"



void start_msh(){
    char ready[] = "msh> ";
    int lenReady = 5;
    int running = 1;
    int done = 1;
    const int LINE_BUFFER_CAPACITY = 20; 
    uint32_t* byte; 

    struct line_buffer{
        uint32_t buffer[LINE_BUFFER_CAPACITY]; 
        uint32_t next_write; //tracks the next byte (index) to be written in
    };


    //init line buffer
    struct line_buffer lb;
    lb.next_write = 0;

    while(running == 1){
        uart_print(ready, lenReady);

        while(done != 1){
            int fail = readb_uart_rx_buffer(&byte);
            if(fail == 1){ //case where there is nothing to read in the buffer
                continue;
            }
            switch (*byte){
                case '\n':
                //line entered, move to logic for handling it. 
                break;
                case 0x08 || 0x0f: 
                //edit buffer, recieved backspace.
                break;
                default:
                //regular character, write it into the buffer
                if(lb.next_write < 20){
                    lb.buffer[lb.next_write] = *byte;
                    lb.next_write++;
                } 
                //else, the byte is discarded. Sorry :( sucks to suck
                break;
            }
        }
    }

}