#include "type.h"
#include "util.h"
#include "protect.h"
#include "kernel.h"
#include "proto.h"

PUBLIC void init_clock()
{
    outb(TIMER_MODE, RATE_GENERATOR);
    outb(TIMER0, (u8_t) (TIMER_FREQ / HZ));
    outb(TIMER0, (u8_t) ((TIMER_FREQ / HZ) >> 8));

    put_irq_handler(CLOCK_IRQ, clock_handler);
    enable_irq(CLOCK_IRQ);
}
