#ifndef __KERN_EVENT_H___
#define __KERN_EVENT_H___

/**
 * @defgroup event
 * @ingroup kernel
 *
 * @{
 */

/**
 * Initializes the internal variables related to IRQ APIs.
 */
void event_init();

/**
 * Puts the specified Task to event-blocked.
 */
void event_await(int tid, int event);

/**
 * Handles the hardware interrupt that invoked the kernel.
 */
void event_handle();

/** @} */

#endif
