#ifndef MINEMU_SYSCALL_H
#define MINEMU_SYSCALL_H
#ifndef CPSR_MODE_MASK
#define CPSR_MODE_MASK UINT32_C(0x1f)
#endif
#ifndef CPSR_MODE_USR
#define CPSR_MODE_USR UINT32_C(0x10)
#endif
#ifndef CPSR_THUMB
#define CPSR_THUMB UINT32_C(0x20)
#endif
#ifndef A32_SVC_ZERO
#define A32_SVC_ZERO UINT32_C(0xef000000)
#endif 
#ifndef TRACE_USER_SYSCALL
#define TRACE_USER_SYSCALL UINT32_C(0x48454c10)
#endif
#ifndef MAX_USER_IMAGE_PAGES
#define MAX_USER_IMAGE_PAGES 16U
#endif 

#include "minemu/trap.h"

/* Students implement this A32 vector trampoline and its syscall contract. */
void minemu_svc_trampoline(void) __attribute__((noreturn));
struct minemu_trap_frame *minemu_svc_dispatch(struct minemu_trap_frame *frame);

#endif
