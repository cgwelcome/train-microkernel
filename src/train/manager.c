#include <kernel.h>
#include <train/gps.h>
#include <train/manager.h>
#include <train/track.h>
#include <train/trainset.h>
#include <user/clock.h>
#include <user/io.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/scheduler.h>
#include <user/tasks.h>
#include <utils/assert.h>

static TrainIO io;
static int clock_tid;
static int scheduler_tid;
static TrainTrackStatus status;
static const uint32_t present_trains[] = {
    1, 24, 58, 74, 78, 79,
};
static uint32_t present_trains_size = sizeof(present_trains)/sizeof(present_trains[0]);
static const uint32_t initial_velocities[MAX_SPEED_NUM] = {
	0, 190, 180, 170, 160, 160, 150, 140, 140, 104, 84, 78, 64, 50, 46,
};

void trainmanager_init() {
    io.tid = WhoIs(SERVER_NAME_IO);
    io.uart = COM1;
    clock_tid = WhoIs(SERVER_NAME_CLOCK);
    scheduler_tid = WhoIs(SERVER_NAME_SCHEDULER);
	trainset_go(&io);
	trainmanager_init_track(TRAIN_TRACK_B);
	for (uint32_t i = 0; i < MAX_SENSOR_NUM; i++) {
		queue_init(&status.awaitsensors[i]);
	}
	queue_init(&status.initialtrains);
	for (uint32_t id = 0; id < MAX_TRAIN_NUM; id++) {
		Train *train = &status.trains[id];
		train->id = id;
		train->speed = 0;
		train->mode = TRAINMODE_FREE;
		train->active = 0;
		train->next_time = 0;
	}
    for (uint32_t id = 1; id <= 18; id++) {
		status.trainswitches[id].id = id;
    }
    for (uint32_t id = 0x99; id <= 0x9C; id++) {
		status.trainswitches[id].id = id;
    }
	for (uint32_t i = 0; i < present_trains_size; i++) {
		trainset_speed(&io, present_trains[i], 0);
	}
	for (uint32_t i = 0; i < MAX_SPEED_NUM; i++) {
		status.velocities[i] = initial_velocities[i];
	}
	trainmanager_switch_all(DEFAULTSWITCH_STATUS);
}

void trainmanager_init_track(TrainTrackName name) {
    switch (name) {
        case TRAIN_TRACK_A:
            status.track.name = name;
            init_tracka(status.track.nodes);
            break;
        case TRAIN_TRACK_B:
            status.track.name = name;
            init_trackb(status.track.nodes);
            break;
        default:
            throw("unknown track name");
    }
}

static void trainmanager_setup_next(Train *train, uint64_t time) {
	gps_schedule_path(train, &status);
	gps_update_next(train, time, &status);
	/** Check if next position is a sensor */
	/*Printf(io.tid, COM2, "\033[%u;%uH : %s" , 22, 1, train->next_position.src->name);*/
	if (gps_is_sensor(&train->next_position)) {
		/** src or dest is the same */
		Printf(io.tid, COM2, "Next position: %s\n\r" , train->next_position.src->name);
		TrainSensor sensor = gps_node_to_sensor(train->next_position.src);
		queue_push(&status.awaitsensors[trainsensor_hash(&sensor)], (int)train->id);
	}
}

void trainmanager_dispatch_sensor(TrainSensor *sensor, uint64_t time) {
	uint32_t sensor_id = trainsensor_hash(sensor);
    Queue *train_ids = &status.awaitsensors[sensor_id];
	uint32_t train_id;
    if (queue_size(train_ids) > 0) {
		train_id = (uint32_t)queue_pop(train_ids);
    } else if (queue_size(&status.initialtrains) > 0) {
		train_id = (uint32_t)queue_pop(&status.initialtrains);
	}
	else {
		return;
	}
	Train *train = &status.trains[train_id];
	TrainTrackNode *node = &status.track.nodes[sensor_id];
	train->last_position = gps_node_to_edge(node);
	Printf(io.tid, COM2, "Difference: %d\r\n", (uint32_t)(train->next_time-time));
	trainmanager_setup_next(train, time);
}

void trainmanager_speed(uint32_t train_id, uint32_t speed) {
    Train *train = &status.trains[train_id];
    train->speed = speed;
	train->mode = TRAINMODE_FREE;
	if (train->active == 0) {
		queue_push(&status.initialtrains, (int)train_id);
		train->active = 1;
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
	if (train->active == 0) {
		queue_push(&status.initialtrains, (int)train_id);
		train->active = 1;
	}
    trainset_speed(&io, train->id, speed);
}

void trainmanager_reverse(uint32_t train_id) {
    (void)train_id;
}

void trainmanager_switch_all(TrainSwitchStatus switch_status) {
    uint32_t code = 0;
    switch (switch_status) {
        case TRAIN_SWITCH_STRAIGHT:
            code = TRAINSWITCH_STRAIGHT;
            break;
        case TRAIN_SWITCH_CURVED:
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
	TrainRequest request = {
		.type = TRAIN_REQUEST_SWITCH_DONE,
	};
    Schedule(scheduler_tid, TRAINSWITCH_DONE_INTERVAL, MyTid(), (char *)&request, sizeof(request));
}

void trainmanager_switch_one(uint32_t switch_id, TrainSwitchStatus switch_status) {
    switch (switch_status) {
        case TRAIN_SWITCH_STRAIGHT:
            trainset_switch(&io, switch_id, TRAINSWITCH_STRAIGHT);
            break;
        case TRAIN_SWITCH_CURVED:
            trainset_switch(&io, switch_id, TRAINSWITCH_CURVED);
            break;
    }
	status.trainswitches[switch_id].status = switch_status;
    TrainRequest request = {
        .type = TRAIN_REQUEST_SWITCH_DONE,
    };
    Schedule(scheduler_tid, TRAINSWITCH_DONE_INTERVAL, MyTid(), (char *)&request, sizeof(request));
}

void trainmanager_switch_done() {
    trainset_switch_done(&io);
}

void trainmanager_update_status() {
    ActiveTrainSensorList list = trainset_sensor_readall(&io);
    uint32_t time = (uint32_t)Time(clock_tid);
    for (uint32_t i = 0; i < list.size; i++) {
        trainmanager_dispatch_sensor(&list.sensors[i], time);
    }
}

void trainmanager_set_velocity(uint32_t speed, uint32_t velocity) {
	status.velocities[speed] = velocity;
}

void trainmanager_park(uint32_t train_id) {
    (void)train_id;
}

void trainmanager_stop() {
    trainset_stop(&io);
}

void trainmanager_done() {
    for (uint32_t i = 0; i < present_trains_size; i++) {
        trainset_speed(&io, present_trains[i], 0);
    }
}

uint32_t trainsensor_hash(TrainSensor *sensor) {
    return (uint32_t)(sensor->module-'A')*MAX_SENSOR_PER_MODULE+(sensor->id-1);
}
