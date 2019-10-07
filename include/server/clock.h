#ifndef __SERVER_CLOCK_H__
#define __SERVER_CLOCK_H__

#define CLOCK_NOTIFY_INTERVAL 10

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

// InitClockServer() initialize some global counters for the Clock Server.
void InitClockServer();

// CreateClockServer() creates a global Clock Server
int CreateClockServer(unsigned int priority);

#endif
