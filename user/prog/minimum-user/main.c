#include "minemu/user_abi.h"
#include <stdint.h>

void minemu_user_main(void) {
    char hello_world[] ="hello world\n";
    int len = 12;
    ioctl(MINEMU_FD_UART_OUT, MINEMU_IOCTL_UART_WRITE, hello_world, len);
    for (;;) {
        __asm__ volatile("nop");
    }
}
