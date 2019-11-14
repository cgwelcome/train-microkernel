#include <string.h>
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


void Schedule(int tid, int ticks, int target, char *data, size_t data_size) {
    assert(ticks >= 0);
    assert(data_size <= CLOCK_SCHEDULE_DATA_LIMIT);
    // Prepare schedule job
    CSScheduleJob job = {
        .in_use = 1,
        .ticks = ticks,
        .target = target,
        .data_size = data_size,
    };
    memcpy(&(job.data), data, data_size);
    // Send clock request
    CSRequest request = {
        .type = CS_SCHEDULE,
        .data = (int) &job,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}
