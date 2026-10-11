#ifndef MINEMU_IRQ_H
#define MINEMU_IRQ_H

#include "minemu/trap.h"

/* Students implement this A32 vector trampoline and its IRQ dispatch policy. */
void minemu_irq_trampoline(void) __attribute__((noreturn));
struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame);

static inline void minemu_irq_enable(void) {
    __asm__ volatile("cpsie i" : : : "memory");
}

static inline void minemu_irq_disable(void) {
    __asm__ volatile("cpsid i" : : : "memory");
}


// Code generated with assistance from Claude Sonnet 5.5
// Prompt/Context: Discovered it was needed for uart.c during debugging
// Date: October 11, 2026
static inline uint32_t minemu_irq_save(void) {
    uint32_t cpsr;
    __asm__ volatile("mrs %0, cpsr\n\tcpsid i" : "=r"(cpsr) : : "memory");
    return cpsr;
}

static inline void minemu_irq_restore(uint32_t cpsr) {
    if ((cpsr & 0x80U) == 0) {
        __asm__ volatile("cpsie i" : : : "memory");
    }
}

#endif
