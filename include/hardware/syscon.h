#ifndef __HARDWARE_SYSCON_H__
#define __HARDWARE_SYSCON_H__

#define SYSCON_BASE     0x80930000
#define HALT_OFFSET         0x08
#define DEVICE_CFG_OFFSET   0x80
	#define SHENA_MASK      0x01
#define SWLOCK_OFFSET       0xC0
	#define SWLOCK_ON       0x00
	#define SWLOCK_OFF      0xAA

#define ON	1
#define	OFF	0

#include <stdint.h>

/**
 * Toggles lock of the system controller
 */
void syscon_lock(int state);

/**
 * Configures the device
 */
void syscon_config(uint32_t mask);

/**
 * Uses the system controller to halt the system
 */
void syscon_halt();

#endif
