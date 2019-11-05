#include <stdint.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/clock.h>
#include <user/tasks.h>
#include <server/io.h>
#include <server/clock.h>
#include <train/gps.h>
#include <train/job.h>
#include <train/manager.h>
#include <train/trainset.h>
#include <train/track.h>

static TrainIO io;
static int clocktid;
static TrainTrackStatus status;
static Queue awaitsensors[MAX_SENSOR_NUM];
static const uint32_t active_trains[] = {
	1, 24, 58, 74, 78, 79,
};
static uint32_t active_trains_size = sizeof(active_trains)/sizeof(active_trains[0]);

void trainmanager_init() {
	io.tid = WhoIs(IO_SERVER_NAME);
	io.uart = COM1;
	clocktid = WhoIs(CLOCK_SERVER_NAME);
	for (uint32_t i = 0; i < MAX_SENSOR_NUM; i++) {
		queue_init(&awaitsensors[i]);
	}
	for (uint32_t i = 0; i < active_trains_size; i++) {
		uint32_t id = active_trains[i];
		Train *train = &status.trains[id];
		train->id = id;
		train->speed = 0;
		trainset_speed(&io, train->id, train->speed);
	}
}

void trainmanager_init_track(TrainTrackType type) {
	switch (type) {
		case TRAINTRACKTYPE_A:
			status.track.type = type;
			init_tracka(status.track.nodes);
			break;
		case TRAINTRACKTYPE_B:
			status.track.type = type;
			init_trackb(status.track.nodes);
			break;
	}
}

static void trainmanager_schedule(TrainJob *job) {
    tjqueue_push(&status.jobqueue, job);
    Create(2900, &trainjob_notifier_task);
}

static void trainmanager_setup_next(Train *train, uint32_t time) {
	traingps_update_next(train, time, &status);
	TrainJobQueue jobs = traingps_next_jobs(train, &status);
	while (tjqueue_size(&jobs) > 0) {
		TrainJob job = tjqueue_pop(&jobs);
		trainmanager_schedule(&job);
	}
	/** Train wait on a sensor */
	if (traingps_is_sensor(&train->next_position)) {
		TrainSensor sensor = traingps_node_to_sensor(&train->next_position.base);
		queue_push(&awaitsensors[trainsensor_hash(&sensor)], (int)train->id);
	}
}

void trainmanager_dispatch_sensor(TrainSensor *sensor, uint32_t time) {
	Queue train_ids = awaitsensors[trainsensor_hash(sensor)];
	while (queue_size(&train_ids) > 0) {
		Train train = status.trains[queue_pop(&train_ids)];
		trainmanager_setup_next(&train, time);
	}
}

void trainmanager_speed(uint32_t train_id, uint32_t speed) {
	Train *train = &status.trains[train_id];
	train->mode = TRAINMODE_FREE;
	train->speed = speed;
	uint32_t time = (uint32_t)Time(clocktid);
	trainmanager_setup_next(train, time);
	trainset_speed(&io, train->id, train->speed);
}

void trainmanager_move(uint32_t train_id, uint32_t speed, uint32_t node_id, int32_t offset) {
	Train *train = &status.trains[train_id];
	TrainTrack *track = &status.track;
	TrainPosition dest = {
		.base = track->nodes[node_id],
		.offset = offset,
	};

	train->path = traingps_find(&train->last_position, &dest, &status);
	train->mode = TRAINMODE_PATH;
	train->speed = speed;

	uint32_t time = (uint32_t)Time(clocktid);
	trainmanager_setup_next(train, time);
	trainset_speed(&io, train->id, speed);
}

void trainmanager_reverse(uint32_t train_id) {
	(void)train_id;
}

void trainmanager_switch_all(TrainSwitchStatus status) {
	uint32_t code = 0;
	switch (status) {
		case TRAINSWITCHSTATUS_STRAIGHT:
			code = TRAINSWITCH_STRAIGHT;
			break;
		case TRAINSWITCHSTATUS_CURVED:
			code = TRAINSWITCH_CURVED;
			break;
	}
    for (uint32_t id = 1; id <= 18; id++) {
        trainset_switch(&io, id, code);
    }
    for (uint32_t id = 0x99; id <= 0x9C; id++) {
        trainset_switch(&io, id, code);
    }
	TMRequest request = {
		.type = TMREQUESTTYPE_SWITCH_DONE,
	};
	TrainJob job = create_trainjob(request, TRAINSWITCH_DONE_INTERVAL);
	trainmanager_schedule(&job);
}

void trainmanager_switch_one(uint32_t switch_id, TrainSwitchStatus status) {
	switch (status) {
		case TRAINSWITCHSTATUS_STRAIGHT:
			trainset_switch(&io, switch_id, TRAINSWITCH_STRAIGHT);
			break;
		case TRAINSWITCHSTATUS_CURVED:
			trainset_switch(&io, switch_id, TRAINSWITCH_CURVED);
			break;
	}
	TMRequest request = {
		.type = TMREQUESTTYPE_SWITCH_DONE,
	};
	TrainJob job = create_trainjob(request, TRAINSWITCH_DONE_INTERVAL);
	trainmanager_schedule(&job);
}

void trainmanager_switch_done() {
	trainset_switch_done(&io);
}

void trainmanager_update_status() {
	ActiveTrainSensorList list = trainset_sensor_readall(&io);
	uint32_t time = (uint32_t)Time(clocktid);
	for (uint32_t i = 0; i < list.size; i++) {
		trainmanager_dispatch_sensor(&list.sensors[i], time);
	}
}

void trainmanager_init_job(int32_t tid) {
    TrainJob job = tjqueue_pop(&status.jobqueue);
    Reply(tid, (char *)&job, sizeof(job));
}

void trainmanager_park(uint32_t train_id) {
	(void)train_id;
}

void trainmanager_stop() {
	trainset_stop(&io);
}

void trainmanager_done() {
	for (uint32_t i = 0; i < active_trains_size; i++) {
		trainset_speed(&io, active_trains[i], 0);
	}
}

uint32_t trainsensor_hash(TrainSensor *sensor) {
	return (uint32_t)(sensor->module-'A')*MAX_SENSOR_PER_MODULE+(sensor->id-1);
}
