#include <train/gps.h>
#include <train/track.h>
#include <utils/bwio.h>
#include <utils/assert.h>
#include <user/tasks.h>

void traingps_test_find() {
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

void traingps_test_subpath() {
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

void traingps_test_middle_free() {
	Train train;
    TrainTrackStatus status;
	init_trackb(status.track.nodes);
    TrainTrackEdge dest = traingps_node_to_edge(&status.track.nodes[64]);
	train.last_position = dest;
    for (uint32_t id = 1; id <= 18; id++) {
		status.trainswitches[id].id = id;
		status.trainswitches[id].status = TRAINSWITCHSTATUS_CURVED;
    }
    for (uint32_t id = 0x99; id <= 0x9C; id++) {
		status.trainswitches[id].id = id;
		status.trainswitches[id].status = TRAINSWITCHSTATUS_CURVED;
    }
	TrainTrackEdge edge = traingps_next_dest_free(&train, &status);
	bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
	bwprintf(COM2, "\n\r");
	status.trainswitches[154].id = 154;
	status.trainswitches[154].status = TRAINSWITCHSTATUS_STRAIGHT;
	edge = traingps_next_dest_free(&train, &status);
	bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
	bwprintf(COM2, "\n\r");
	status.trainswitches[153].id = 153;
	status.trainswitches[153].status = TRAINSWITCHSTATUS_STRAIGHT;
	edge = traingps_next_dest_free(&train, &status);
	bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
	bwprintf(COM2, "\n\r");
	status.trainswitches[154].id = 154;
	status.trainswitches[154].status = TRAINSWITCHSTATUS_CURVED;
	edge = traingps_next_dest_free(&train, &status);
	bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
	bwprintf(COM2, "\n\r");
	Exit();
}

void traingps_test_next_dest_free() {
    TrainSensor src_sensor = {
        .id = 13,
        .module = 'A',
    };
	Train train;
    TrainTrackStatus status;
	init_trackb(status.track.nodes);
    TrainTrackNode *node = &status.track.nodes[trainsensor_hash(&src_sensor)];
    TrainTrackEdge dest = traingps_node_to_edge(node);
	train.last_position = dest;
    for (uint32_t id = 1; id <= 18; id++) {
		status.trainswitches[id].id = id;
		status.trainswitches[id].status = TRAINSWITCHSTATUS_CURVED;
    }
	TrainTrackEdge edge = traingps_next_dest_free(&train, &status);
	bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
	bwprintf(COM2, "\n\r");
	Exit();
}

void traingps_test_root_task() {
	/*Create(1, &traingps_test_find);*/
    /*Create(2, &traingps_test_subpath);*/
    Create(2, &traingps_test_next_dest_free);
	Exit();
}
