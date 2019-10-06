#include <server/clock.h>
#include <user/ipc.h>

int Time(int tid) {
    int tick;
    CSRequest request = {
        .type = CS_TIME,
    };
    int status = Send(tid, (char *)&request, sizeof(request),
            (char *)&tick, sizeof(tick));
    if (status < 0) return -1;
    return tick;
}

int Delay(int tid, int ticks) {
    if (ticks < 0) return -2;
    int tick;
    CSRequest request = {
        .type = CS_DELAY,
        .data = ticks,
    };
    int status = Send(tid, (char *)&request, sizeof(request),
            (char *)&tick, sizeof(tick));
    if (status < 0) return -1;
    return tick;
}

int DelayUntil(int tid, int ticks) {
    if (ticks < 0) return -2;
    int tick;
    CSRequest request = {
        .type = CS_DELAYUNTIL,
        .data = ticks,
    };
    int status = Send(tid, (char *)&request, sizeof(request),
            (char *)&tick, sizeof(tick));
    if (status < 0) return -1;
    return tick;
}
