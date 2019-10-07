#ifndef __HARDWARE_SYSCON_H__
#define __HARDWARE_SYSCON_H__

#define SYSCON_BASE     0x80930000
#define HALT_OFFSET         0x8
#define DEVICE_CFG_OFFSET   0x80
	#define SHENA_MASK      0x1
#define SWLOCK_OFFSET       0xC0
	#define SWLOCK_ON       0x0
	#define SWLOCK_OFF      0xAA

#define ON	1
#define	OFF	0

// syscon_lock() toggles lock of the system controller
void syscon_lock(int state);

// syscon_config() configures the device
void syscon_config(unsigned int mask);

// syscon_halt() uses the system controller to halt the system
void syscon_halt(); 

#endif
