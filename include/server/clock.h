#ifndef __SERVER_CLOCK_H__
#define __SERVER_CLOCK_H__

#define TIMER_IRQ_INTERVAL 10

typedef enum {
    CS_TICKUPDATE,
    CS_DELAY,
    CS_DELAYUNTIL,
    CS_TIME,
} CSRequestType;

typedef struct {
    CSRequestType type;
    int data;
} CSRequest;

int CreateCS(unsigned int priority);

#endif 
