#ifndef __KERN_IRQ_H___
#define __KERN_IRQ_H___

// irq_init() initializes the internal variables related to IRQ APIs.
void irq_init();

// irq_await() puts the tid Task to eventblocked
void irq_await(int tid, int event);

// irq_unblock() unblocks the awaiting task on the event
void irq_unblock(int event);

// irq_handle() handles the hardware interrupt that has triggered the kerne to
// context switch
void irq_handle();

#endif
