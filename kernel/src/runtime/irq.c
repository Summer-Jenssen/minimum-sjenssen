#include "minemu/boot.h"
#include "minemu/irq.h"
#include "minemu/platform.h"
#include "minemu/uart.h"

#define NUM_PERIPHERALS 4 //may need to edit at a later date
typedef void (*handler)(struct minemu_trap_frame *frame);
static handler function_table[NUM_PERIPHERALS] = {NULL, &uart_handler, NULL, NULL};


struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame){
    uint32_t source = (uint32_t)frame->exception_id;

    if((int)source >= 0 && (int)source < NUM_PERIPHERALS ){
        if(function_table[(int)source] != NULL){
            (*function_table[(int)source])(frame);
        }
    }

    MINEMU_INTERRUPT->eoi = source;
    return frame;
}