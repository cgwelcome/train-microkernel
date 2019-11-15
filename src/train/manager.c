#include <kernel.h>
#include <train/gps.h>
#include <train/manager.h>
#include <train/track.h>
#include <train/trains.h>
#include <train/trainset.h>
#include <user/io.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/scheduler.h>
#include <user/tasks.h>
#include <user/ui.h>
#include <utils/assert.h>

#include <hardware/timer.h>

static TrainIO io;
static int scheduler_tid;

Queue initial_trains;
Queue await_sensors[MAX_SENSOR_NUM];
ActiveTrainSensorList sensor_log;

void trainmanager_init() {
    io.tid = WhoIs(SERVER_NAME_IO);
    io.uart = COM1;
    scheduler_tid = WhoIs(SERVER_NAME_SCHEDULER);
    trains_init();
    queue_init(&initial_trains);
    for (size_t i = 0; i < MAX_SENSOR_NUM; i++) {
        queue_init(&await_sensors[i]);
    }
    sensor_log.size = 0;
	trainset_go(&io);
	trainset_park_all(&io);
	trainmanager_switch_all(DIR_CURVED);
}

void trainmanager_init_track(TrackName name) {
    track_init(name);
}

void trainmanager_start(uint32_t train_id) {
    queue_push(&initial_trains, (int) train_id);
    trainset_speed(&io, train_id, 10);
}

void trainmanager_speed(uint32_t train_id, uint32_t speed) {
    trains_set_speed(train_id, speed);
    trainset_speed(&io, train_id, speed);
}

// void trainmanager_move(uint32_t train_id, uint32_t speed, uint32_t node_id, uint32_t offset) {
//     Train *train = &status.trains[train_id];
// 	TrackEdge dest = {
// 		.src = track_find_node(node_id),
// 		.dest = track_find_node(node_id),
// 		.dist = offset,
// 	};
//     train->speed = speed;
// 	train->mode = TRAINMODE_PATH;
// 	if (train->last_position.src == NULL) {
// 		queue_push(&status.initialtrains, (int)train_id);
// 	} else {
// 		train->path = gps_find(&train->last_position, &dest, &status);
// 		uint32_t time = (uint32_t)Time(clock_tid);
// 		trainmanager_setup_next(train, time);
// 	}
//     trainset_speed(&io, train->id, speed);
// }

void trainmanager_reverse(uint32_t train_id) {
    (void)train_id;
}

void trainmanager_switch_all(int8_t switch_status) {
    uint32_t code = 0;
    switch (switch_status) {
        case DIR_STRAIGHT:
            code = TRAINSWITCH_STRAIGHT;
            break;
        case DIR_CURVED:
            code = TRAINSWITCH_CURVED;
            break;
        default:
            throw("unknow switch status");
    }
    for (uint32_t id = 1; id <= 18; id++) {
		trainset_switch(&io, id, code);
        track_set_branch_direction(id, switch_status);
        PrintSwitch(io.tid, id, switch_status);
    }
    for (uint32_t id = 0x99; id <= 0x9C; id++) {
        trainset_switch(&io, id, code);
		track_set_branch_direction(id, switch_status);
        PrintSwitch(io.tid, id, switch_status);
    }
	TrainRequest request = {
		.type = TRAIN_REQUEST_SWITCH_DONE,
	};
    Schedule(scheduler_tid, TRAINSWITCH_DONE_INTERVAL, MyTid(), (char *)&request, sizeof(request));
}

void trainmanager_switch_one(uint32_t switch_id, int8_t switch_status) {
    switch (switch_status) {
        case DIR_STRAIGHT:
            trainset_switch(&io, switch_id, TRAINSWITCH_STRAIGHT);
            break;
        case DIR_CURVED:
            trainset_switch(&io, switch_id, TRAINSWITCH_CURVED);
            break;
        default:
            throw("unknow switch status");
    }
    track_set_branch_direction(switch_id, switch_status);
    PrintSwitch(io.tid, switch_id, switch_status);
    TrainRequest request = {
        .type = TRAIN_REQUEST_SWITCH_DONE,
    };
    Schedule(scheduler_tid, TRAINSWITCH_DONE_INTERVAL, MyTid(), (char *)&request, sizeof(request));
}

void trainmanager_switch_done() {
    trainset_switch_done(&io);
}

static void trainmanager_await_next_sensor(uint32_t train_id, TrackNode *sensor) {
    assert(sensor != NULL);
    TrackNode *next_sensor = track_find_next_sensor(sensor);
    if (next_sensor != NULL) {
        queue_push(&await_sensors[next_sensor->num], (int) train_id);
    }
}

static void trainmanager_touch_train_sensor(uint32_t train_id, TrackNode *sensor) {
    Train *train = trains_find(train_id);
    if (train != NULL && train->position.node != NULL) {
        PrintTimeDifference(io.tid, train_id, train->next_sensor_expected_time);
        uint32_t now = timer_read(TIMER3);
        uint32_t time = now - (uint32_t) train->prev_touch_time;
        TrackNode *next = track_find_next_sensor(train->prev_touch_node);
        uint32_t dist = track_find_next_sensor_dist(next);
        Printf(io.tid, COM2, "\033[30;1H\033[K%s %u %u", sensor->name, time, dist * 1000 / time);
    }
    trains_touch_sensor(train_id, sensor);
    trainmanager_await_next_sensor(train_id, sensor);
}

static void trainmanager_update_log(TrainSensor *sensor, Train *train) {
    if (sensor_log.size < MAX_SENSOR_LOG) {
        sensor_log.size += 1;
    }
    for (int t = MAX_SENSOR_LOG - 1; t > 0; t--) {
        sensor_log.sensors[t] = sensor_log.sensors[t - 1];
        sensor_log.sensors[t].train = train;
    }
    sensor_log.sensors[0] = *sensor;
    sensor_log.sensors[0].train = train;
}

static void trainmanager_update_check_sensors() {
    ActiveTrainSensorList list = trainset_sensor_readall(&io);
    for (uint32_t i = 0; i < list.size; i++) {
        TrackNode *sensor = track_find_sensor(list.sensors[i].module, list.sensors[i].id);
        Train *train = NULL;
        /*if (sensor == NULL) {*/
        /*} else if (queue_size(&initial_trains) > 0) {*/
            /*uint32_t train_id = (uint32_t) queue_pop(&initial_trains);*/
            /*trainmanager_touch_train_sensor(train_id, sensor);*/
        /*} else if (queue_size(&await_sensors[sensor->num]) > 0) {*/
            /*uint32_t train_id = (uint32_t) queue_pop(&await_sensors[sensor->num]);*/
            /*trainmanager_touch_train_sensor(train_id, sensor);*/
        /*}*/
        trainmanager_update_log(&list.sensors[i], train);
    }
    PrintSensors(io.tid, &sensor_log);
}

void trainmanager_update_status() {
    trainmanager_update_check_sensors();
    /*for (int i = 0; i < TRAIN_COUNT; i++) {*/
        /*trains_estimite_position(i);*/
    /*}*/
}

void trainmanager_park(uint32_t train_id) {
    (void)train_id;
}

void trainmanager_stop() {
    trainset_stop(&io);
}

void trainmanager_done() {
    trainset_park_all(&io);
}

uint32_t trainsensor_hash(TrainSensor *sensor) {
    return (uint32_t)(sensor->module-'A')*MAX_SENSOR_PER_MODULE+(sensor->id-1);
}
