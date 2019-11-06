#include <string.h>
#include <server/scheduler.h>
#include <user/scheduler.h>
#include <user/ipc.h>
#include <utils/assert.h>

void Schedule(int tid, int ticks, int target, char *data, size_t data_size) {
    assert(ticks >= 0);
    assert(data_size <= SCHEDULER_CALLBACK_DATA_SIZE);
    SSRequest request = {
        .type = SS_SCHEDULE,
        .ticks = ticks,
        .target = target,
        .data_size = data_size,
    };
    memcpy(&(request.data), data, data_size);
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}
