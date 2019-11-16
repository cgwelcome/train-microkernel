#include <server/clock.h>
#include <user/clock.h>
#include <user/ipc.h>
#include <utils/assert.h>

int Time(int tid) {
    CSRequest request = {
        .type = CS_TIME,
    };
    int time;
    assert(Send(tid, (char *)&request, sizeof(request), (char *)&time, sizeof(time)) >= 0);
    return time;
}

int Delay(int tid, int ticks) {
    assert(ticks >= 0);
    CSRequest request = {
        .type = CS_DELAY,
        .data = ticks,
    };
    int time;
    assert(Send(tid, (char *)&request, sizeof(request), (char *)&time, sizeof(time)) >= 0);
    return time;
}

int DelayUntil(int tid, int ticks) {
    assert(ticks >= 0);
    CSRequest request = {
        .type = CS_DELAYUNTIL,
        .data = ticks,
    };
    int time;
    assert(Send(tid, (char *)&request, sizeof(request), (char *)&time, sizeof(time)) >= 0);
    return time;
}
