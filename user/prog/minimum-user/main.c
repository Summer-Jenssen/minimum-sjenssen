#include "minemu/user_abi.h"
#include "msh.h"
#include <stdint.h>

void minemu_user_main(void) {
    // char hello_world[] ="hello world\n";
    // int len = 12;
    // ioctl(MINEMU_FD_UART_OUT, MINEMU_IOCTL_UART_WRITE, hello_world, len);
    // char c;
    // for (;;) {
    //     ioctl(MINEMU_FD_UART_OUT, MINEMU_IOCTL_UART_READ, &c, 1);
    //     for (volatile int i = 0; i < 2000; i++) { }
    //  }
    start_msh();
}
