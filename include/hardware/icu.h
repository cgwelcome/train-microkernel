#ifndef __HARDWARE_ICU_H__
#define __HARDWARE_ICU_H__

#define VIC1_BASE 0x800B0000
#define VIC2_BASE 0x800C0000

#define IRQSTATUS_OFFSET        0x00
#define INTSELECT_OFFSET        0x0C
#define INTENABLE_OFFSET        0x10
#define INTENCLEAR_OFFSET       0x14
#define SOFTINT_OFFSET          0x18
#define SOFTINTCLEAR_OFFSET     0x1C
#define PROTECTION_OFFSET       0x20

/**
 * Initializes the interrupt control unit variables.
 */
void icu_init(void);

/**
 * Activates an event interrupt.
 */
void icu_activate(int event);

/**
 * Activates a interrupt of a event. Used for debugging.
 */
void icu_softirq(int event);

/**
 * Reads the status register of interrupt control unit to find current event.
 */
int icu_read();

/**
 * Clears an active interrupt, reverses the icu_active() function
 */
void icu_disable(int event);

void icu_disableall();

#endif /*__HARDWARE_ICU_H__*/
