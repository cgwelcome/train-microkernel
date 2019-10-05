#ifndef __UTILS_ICU_H__
#define __UTILS_ICU_H__

// icu_init() initialize the interrupt control unit variables.
void icu_init(void);

// icu_activate() activate an event interrupt.
void icu_activate(unsigned int event);

// icu_softirq() activate a interrupt of a event. Used for debugging
void icu_softirq(unsigned int event);

// icu_activeirq() checks if an event has been activate
// Return Values:
//  0: False
//  1: True
int icu_activeirq(unsigned int event);

// icu_clear() clears an active interrupt, reverses the icu_active() function
void icu_clear(unsigned int event);

#endif
