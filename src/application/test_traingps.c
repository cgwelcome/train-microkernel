#include <train/gps.h>
#include <train/track.h>
#include <utils/bwio.h>
#include <user/tasks.h>

void traingps_test_root_task() {
    TrainTrackStatus status;
    init_trackb(status.track.nodes);
    TrainPosition src = {
        .base = status.track.nodes[0], // A1
        .offset = 0,
    };
    TrainPosition dest = {
        .base = status.track.nodes[64], // E1
        .offset = 0,
    };
    TrainPath path = traingps_find(&src, &dest, &status);
    for (uint32_t i = 0; i < path.size; i++) {
        bwprintf(COM2, "%s -> ", path.nodes[i].name);
    }
    Exit();
}
