#include <stdint.h>
#include <kernel.h>
#include <train/gps.h>
#include <train/job.h>
#include <train/manager.h>
#include <train/track.h>
#include <train/trainset.h>
#include <user/clock.h>
#include <user/io.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>

static TrainIO io;
static int clocktid;
static TrainTrackStatus status;
static const uint32_t active_trains[] = {
    1, 24, 58, 74, 78, 79,
};
static uint32_t active_trains_size = sizeof(active_trains)/sizeof(active_trains[0]);

void trainmanager_init() {
    io.tid = WhoIs(SERVER_NAME_IO);
    io.uart = COM1;
    clocktid = WhoIs(SERVER_NAME_CLOCK);
	trainset_go(&io);
	trainmanager_init_track(TRAINTRACKTYPE_B);
	tjqueue_init(&status.jobqueue);
	for (uint32_t i = 0; i < MAX_SENSOR_NUM; i++) {
		queue_init(&status.awaitsensors[i]);
	}
	queue_init(&status.initialtrains);
	for (uint32_t id = 0; id < MAX_TRAIN_NUM; id++) {
		Train *train = &status.trains[id];
		train->id = id;
		train->speed = 0;
		train->last_position.src = NULL;
		train->mode = TRAINMODE_FREE;
	}
    for (uint32_t id = 1; id <= 18; id++) {
		status.trainswitches[id].id = id;
    }
    for (uint32_t id = 0x99; id <= 0x9C; id++) {
		status.trainswitches[id].id = id;
    }
	for (uint32_t i = 0; i < active_trains_size; i++) {
		trainset_speed(&io, active_trains[i], 0);
	}
	trainmanager_switch_all(DEFAULTSWITCH_STATUS);
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
    Create(PRIORITY_NOTIFIER_TMS_DELAY, &trainjob_notifier_task);
}

static void trainmanager_setup_next(Train *train, uint32_t time) {
	TrainJobQueue jobs = traingps_next_jobs(train, &status);
	while (tjqueue_size(&jobs) > 0) {
		TrainJob job = tjqueue_pop(&jobs);
		trainmanager_schedule(&job);
	}
	traingps_update_next(train, time, &status);
	/** Check if next position is a sensor */
	Printf(io.tid, COM2, "\033[%u;%uH : %s" , 22, 1, train->next_position.src->name);
	if (traingps_is_sensor(&train->next_position)) {
		/** src or dest is the same */
		TrainSensor sensor = traingps_node_to_sensor(train->next_position.src);
		Printf(io.tid, COM2, "\033[%u;%uH : %c" , 20, 1, sensor.module);
		Printf(io.tid, COM2, "\033[%u;%uH : %u" , 21, 1, sensor.id);
		queue_push(&status.awaitsensors[trainsensor_hash(&sensor)], (int)train->id);
	}
}

void trainmanager_dispatch_sensor(TrainSensor *sensor, uint32_t time) {
	uint32_t sensor_id = trainsensor_hash(sensor);
    Queue train_ids = status.awaitsensors[sensor_id];
	uint32_t train_id;
    if (queue_size(&train_ids) > 0) {
		train_id = (uint32_t)queue_pop(&train_ids);
    } else if (queue_size(&status.initialtrains) > 0) {
		train_id = (uint32_t)queue_pop(&status.initialtrains);
	}
	else {
		return;
	}
	Train *train = &status.trains[train_id];
	TrainTrackNode *node = &status.track.nodes[sensor_id];
	train->last_position = traingps_node_to_edge(node);
	trainmanager_setup_next(train, time);
}

void trainmanager_speed(uint32_t train_id, uint32_t speed) {
    Train *train = &status.trains[train_id];
    train->speed = speed;
	train->mode = TRAINMODE_FREE;
	if (train->last_position.src == NULL) {
		queue_push(&status.initialtrains, (int)train_id);
	} else {
		uint32_t time = (uint32_t)Time(clocktid);
		trainmanager_setup_next(train, time);
	}
    trainset_speed(&io, train->id, train->speed);
}

void trainmanager_move(uint32_t train_id, uint32_t speed, uint32_t node_id, uint32_t offset) {
    Train *train = &status.trains[train_id];
	TrainTrackEdge dest = {
		.src = &status.track.nodes[node_id],
		.dest = &status.track.nodes[node_id],
		.dist = offset,
	};
    train->speed = speed;
	train->mode = TRAINMODE_PATH;
	if (train->last_position.src == NULL) {
		queue_push(&status.initialtrains, (int)train_id);
	} else {
		train->path = traingps_find(&train->last_position, &dest, &status);
		uint32_t time = (uint32_t)Time(clocktid);
		trainmanager_setup_next(train, time);
	}
    trainset_speed(&io, train->id, speed);
}

void trainmanager_reverse(uint32_t train_id) {
    (void)train_id;
}

void trainmanager_switch_all(TrainSwitchStatus switch_status) {
    uint32_t code = 0;
    switch (switch_status) {
        case TRAINSWITCHSTATUS_STRAIGHT:
            code = TRAINSWITCH_STRAIGHT;
            break;
        case TRAINSWITCHSTATUS_CURVED:
            code = TRAINSWITCH_CURVED;
            break;
    }
    for (uint32_t id = 1; id <= 18; id++) {
		status.trainswitches[id].status = switch_status;
		trainset_switch(&io, id, code);
    }
    for (uint32_t id = 0x99; id <= 0x9C; id++) {
		status.trainswitches[id].status = switch_status;
        trainset_switch(&io, id, code);
    }
	TMRequest request = {
		.type = TM_REQUEST_SWITCH_DONE,
	};
	TrainJob job = create_trainjob(request, TRAINSWITCH_DONE_INTERVAL);
	trainmanager_schedule(&job);
}

void trainmanager_switch_one(uint32_t switch_id, TrainSwitchStatus switch_status) {
    switch (switch_status) {
        case TRAINSWITCHSTATUS_STRAIGHT:
            trainset_switch(&io, switch_id, TRAINSWITCH_STRAIGHT);
            break;
        case TRAINSWITCHSTATUS_CURVED:
            trainset_switch(&io, switch_id, TRAINSWITCH_CURVED);
            break;
    }
	status.trainswitches[switch_id].status = switch_status;
    TMRequest request = {
        .type = TM_REQUEST_SWITCH_DONE,
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
		Printf(io.tid, COM2, "\033[%u;%uH : %c" , 17, 1, list.sensors[i].module);
		Printf(io.tid, COM2, "\033[%u;%uH : %u" , 18, 1, list.sensors[i].id);
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
