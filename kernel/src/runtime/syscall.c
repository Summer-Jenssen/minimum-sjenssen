#include "minemu/syscall.h"
#include "minemu/platform.h"
#include "minemu/boot.h"
#include "minemu/syscall.h"
#include "minemu/user_abi.h"
#include "minemu/uart.h"
#include "minemu/mmu.h"
#include "minemu/trace.h"



struct minemu_trap_frame *minemu_svc_dispatch(struct minemu_trap_frame *frame){
    uint32_t mode = frame->spsr & CPSR_MODE_MASK;
    if (frame->exception_id != MINEMU_EXCEPTION_SVC || mode != CPSR_MODE_USR ||
        (frame->spsr & CPSR_THUMB) != 0) {
        frame->r[0] = (uint32_t)-1; //error
        return frame;
    }

    uint32_t instruction = *(const uint32_t *)(uintptr_t)frame->fault_pc;


    if (instruction == A32_SVC_ZERO){

        if (frame->r[7] == MINEMU_SYSCALL_IOCTL){
            uint32_t fd = frame->r[0];
            uint32_t operation = frame->r[1];
            if (operation == MINEMU_IOCTL_UART_WRITE){
                if( fd == MINEMU_FD_UART_OUT || fd == MINEMU_FD_UART_IN){
                    uint32_t arg1 = frame->r[3];
                    if (validate_mem_range(frame->r[2], arg1, 0, 1) == 1){
                        uint32_t nbytes_written = uart_print((char*)(uintptr_t) frame->r[2], arg1, fd);
                        if(nbytes_written == 0){ //failure :(
                            frame->r[0] = (uint32_t)-1;
                        } else { //we wrote at least something
                            frame->r[0] = nbytes_written;
                        }
                    } else {
                        frame->r[0] = (uint32_t)-1;
                    }
                } else {
                    frame->r[0] = (uint32_t)-1;
                }

            } else if (operation == MINEMU_IOCTL_UART_READ){
                if( fd == MINEMU_FD_UART_OUT || fd == MINEMU_FD_UART_IN){
                    uint32_t* arg0 = (uint32_t*) (uintptr_t) frame->r[2];
                    uint32_t arg1 = frame->r[3];
                    if(validate_mem_range(frame->r[2], arg1, 1, 0) == 1){
                        uint32_t nbytes_read = 0;
                        for(int i = 0; (uint32_t)i < arg1; i++){
                            int ret_val = readb_uart_rx_buffer(&arg0[i], fd);
                            if(ret_val == 0){
                                nbytes_read++;
                            }
                        }
                        if(nbytes_read == 0){ //failure :(
                            frame->r[0] = (uint32_t)-1;
                        } else { //we wrote at least something
                            frame->r[0] = nbytes_read;
                        }
                    } else {
                        frame->r[0] = (uint32_t)-1;
                    }
                } else {
                    frame->r[0] = (uint32_t)-1;
                }
            } else if (operation == MINEMU_IOCTL_RNG_SEED){
                if (fd == MINEMU_FD_RNG){
                uint32_t arg0 = frame->r[2]; //new seed
                MINEMU_RNG->seed = arg0;
                frame->r[0] = 0;
                } else {
                    frame->r[0] = (uint32_t)-1;
                }
            } else if (operation == MINEMU_IOCTL_RNG_NEXT){
                if(fd == MINEMU_FD_RNG){
                    uint32_t* arg0 = (uint32_t*) (uintptr_t) frame->r[2]; //ptr to put data into
                    if (validate_mem_range(frame->r[2], sizeof(uint32_t), 1, 0) == 1){
                        *arg0 = MINEMU_RNG->data;
                        frame->r[0] = 0;
                    } else {
                        frame->r[0] = (uint32_t)-1;
                    }
                } else {
                    frame->r[0] = (uint32_t)-1;
                }
            } else if (operation == MINEMU_IOCTL_TRACE_EVENT){
                if(fd == MINEMU_FD_TRACE){
                    uint32_t arg0 = frame->r[2];
                    minemu_trace_event(arg0);
                    frame->r[0] = 0;
                } else {
                    frame->r[0] = (uint32_t)-1;
                }
            }
        }
    } else {
        frame->r[0] = (uint32_t)-1;
    }
    //write result into saved copy of r0
    //return and pass it back over to the trampoline

    return frame;
}