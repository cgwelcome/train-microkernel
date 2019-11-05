#include <train/gps.h>
#include <train/track.h>
#include <utils/bwio.h>
#include <utils/assert.h>
#include <user/tasks.h>

static void traingps_test_find() {
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
	TrainPosition src = {
		.base = status.track.nodes[trainsensor_hash(&src_sensor)], // A1
		.offset = 0,
	};
	TrainPosition dest = {
		.base = status.track.nodes[trainsensor_hash(&dest_sensor)], // A1
		.offset = 132,
	};
	TrainPath path = traingps_find(&src, &dest, &status);
	for (uint32_t i = 0; i < path.size; i++) {
		bwprintf(COM2, "%s -> ", path.nodes[i].name);
	}
	bwprintf(COM2, "\n\r");
	TrainPath subpath = traingps_get_subpath(&path, 1500);
	for (uint32_t i = 0; i < subpath.size; i++) {
		bwprintf(COM2, "%s -> ", subpath.nodes[i].name);
	}
	bwprintf(COM2, "\n\r");
	for (uint32_t i = 0; i < 13; i++) {
		TrainPosition position = traingps_next_relative(&path);
		bwprintf(COM2, "%s-%d", position.base.name, position.offset);
		bwprintf(COM2, "\n\r");
	}
	Exit();
}

void traingps_test_root_task() {
	/*Create(0, &traingps_test_find);*/
	Create(1, &traingps_test_subpath);
}
