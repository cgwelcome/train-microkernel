#ifndef __KERN_EVENT_H___
#define __KERN_EVENT_H___

// event_init() initializes the internal variables related to IRQ APIs.
void event_init();

// event_await() puts the specified Task to event-blocked.
void event_await(int tid, int event);

// event_unblock() unblocks the awaiting task on the event.
void event_unblock(int event);

// event_handle() handles the hardware interrupt that invoked the kernel.
void event_handle();

#endif
