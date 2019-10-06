#ifndef __UTILS_SYSCON_H__
#define __UTILS_SYSCON_H__

// syscon_lock() toggles lock of the system controller
void syscon_lock(int state);

// syscon_config() configures the device
void syscon_config(unsigned int mask);

// syscon_halt() uses the system controller to halt the system
void syscon_halt(); 

#endif
