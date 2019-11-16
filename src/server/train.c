#include <kernel.h>
#include <server/clock.h>
#include <server/train.h>
#include <train/controller.h>
#include <train/train.h>
#include <user/clock.h>
#include <user/tasks.h>
#include <user/ipc.h>
#include <user/name.h>
#include <utils/assert.h>
#include <utils/bwio.h>

// static Queue initial_trains;
// static Queue await_sensors[MAX_SENSOR_NUM];
// static ActiveTrainSensorList sensor_log;

extern Track singleton_track;
extern Train singleton_trains[TRAIN_COUNT];

static void train_server_init() {
    singleton_track.inited = false;
    // queue_init(&initial_trains);
    // for (size_t i = 0; i < MAX_SENSOR_NUM; i++) {
    //     queue_init(&await_sensors[i]);
    // }
    // sensor_log.size = 0;
    for (size_t i = 0; i < TRAIN_COUNT; i++) {
        train_init(&singleton_trains[i], train_index_to_id(i));
    }
}

static void train_root_task() {
    int tid;
    TrainRequest request;

    RegisterAs(SERVER_NAME_TRAIN);
    train_server_init();
    controller_init();
    controller_go(0);
    controller_speed_all(0, 0);
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        if (request.type == TRAIN_REQUEST_INIT_TRACK) {
            uint32_t delay = 0;
            TrackName name = (TrackName) request.args[0];
            track_init(&singleton_track, name);
            delay = controller_switch_all(DIR_CURVED, delay);
            delay = controller_switch_one(6,  DIR_STRAIGHT, delay + 10);
            delay = controller_switch_one(9,  DIR_STRAIGHT, delay + 10);
            delay = controller_switch_one(15, DIR_STRAIGHT, delay + 10);
        }
        if (request.type == TRAIN_REQUEST_INIT_TRAIN) {
            uint32_t train_id = request.args[0];
            // queue_push(&initial_trains, (int) train_id);
            controller_speed_one(train_id, 10, 0);
        }
        if (request.type == TRAIN_REQUEST_WAKE_CONTROLLER) {
            controller_wake();
        }
        if (request.type == TRAIN_REQUEST_LOCATE_TRAINS) {
            // train_manager_locate_trains();
        }
        if (request.type == TRAIN_REQUEST_SPEED) {
            uint32_t train_id = request.args[0];
            uint32_t speed    = request.args[1];
            controller_speed_one(train_id, speed, 0);
        }
        if (request.type == TRAIN_REQUEST_REVERSE) {
            uint32_t train_id = request.args[0];
            uint32_t speed = train_find(singleton_trains, train_id)->speed;
            controller_speed_one(train_id, 0, 0);
            controller_speed_one(train_id, TRAIN_STATUS_REVERSE, CONTROLLER_REVERSE_DELAY);
            controller_speed_one(train_id, speed, CONTROLLER_REVERSE_DELAY + 1);
        }
        if (request.type == TRAIN_REQUEST_MOVE) {
            // driver_navigate(request.args[0], request.args[1], request.args[2], request.args[3]);
        }
        if (request.type == TRAIN_REQUEST_SWITCH) {
            uint32_t switch_id = request.args[0];
            uint32_t direction = request.args[1];
            controller_switch_one(switch_id, direction, 0);
        }
        if (request.type == TRAIN_REQUEST_EXIT) {
            controller_speed_all(0, 0);
        }
        Reply(tid, NULL, 0);
    }
}

void train_notifier_task() {
	int clocktid = WhoIs(SERVER_NAME_CLOCK);
	int traintid = WhoIs(SERVER_NAME_TRAIN);

    TrainRequest wake_request = { .type = TRAIN_REQUEST_WAKE_CONTROLLER };
    TrainRequest locate_request = { .type = TRAIN_REQUEST_LOCATE_TRAINS };

	for (uint32_t i = 0; ; i++) {
		Delay(clocktid, 1);
        if (i % 2 == 0) {
		    assert(Send(traintid, (char *)&wake_request, sizeof(wake_request), NULL, 0) >= 0);
        }
        if (i % 4 == 0) {
		    assert(Send(traintid, (char *)&locate_request, sizeof(locate_request), NULL, 0) >= 0);
        }
	}
	Exit();
}

void CreateTrainServer() {
    Create(PRIORITY_SERVER_TRAIN, &train_root_task);
	Create(PRIORITY_NOTIFIER_TRAIN, &train_notifier_task);
}
