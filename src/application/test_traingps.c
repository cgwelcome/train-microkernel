#include <train/gps.h>
#include <train/track.h>
#include <utils/bwio.h>
#include <utils/assert.h>
#include <user/tasks.h>

static void traingps_test_find() {
    TrainTrackStatus status;
    init_trackb(status.track.nodes);
    TrainTrackEdge src = *status.track.nodes[0].edge;
	src.dist = 0;
    TrainTrackEdge dest = *status.track.nodes[64].edge;
	dest.dist = 0;

    TrainPath path = traingps_find(&src, &dest, &status);
    for (uint32_t i = 0; i < path.size; i++) {
        bwprintf(COM2, "%s -> ", path.nodes[i]->name);
    }
    Exit();
}

static void traingps_test_subpath() {
    TrainSensor src_sensor = {
        .id = 1,
        .module = 'A',
    };
    TrainSensor dest_sensor = {
        .id = 1,
        .module = 'E',
    };
    TrainTrackStatus status;
    init_trackb(status.track.nodes);
    assert(trainsensor_hash(&src_sensor) == 0);
    assert(trainsensor_hash(&dest_sensor) == 64);
    TrainTrackEdge src = *status.track.nodes[0].edge;
	src.dist = 0;
    TrainTrackEdge dest = *status.track.nodes[64].edge;
	dest.dist = 0;
    TrainPath path = traingps_find(&src, &dest, &status);
    for (uint32_t i = 0; i < path.size; i++) {
        bwprintf(COM2, "%s -> ", path.nodes[i]->name);
    }
    bwprintf(COM2, "\n\r");
    for (uint32_t i = 0; i < 12; i++) {
		TrainTrackEdge edge = traingps_next_dest(&path);
        bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
        bwprintf(COM2, "\n\r");
    }
    Exit();
}

void traingps_test_root_task() {
	/*Create(1, &traingps_test_find);*/
    Create(2, &traingps_test_subpath);
}
