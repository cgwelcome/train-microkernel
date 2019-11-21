#include <kernel.h>
#include <hardware/timer.h>
#include <server/clock.h>
#include <server/train.h>
#include <train/controller.h>
#include <train/manager.h>
#include <train/model.h>
#include <train/train.h>
#include <user/clock.h>
#include <user/tasks.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/ui.h>
#include <utils/assert.h>
#include <utils/queue.h>

#define READ_SENSOR_INTERVAL       100 // ms
#define PRINT_STATUS_INTERVAL      200

extern Track singleton_track;
extern Train singleton_trains[TRAIN_COUNT];

static uint32_t last_read_sensor_time;
static uint32_t last_print_status_time;

static void train_server_init() {
    singleton_track.inited = false;
    for (size_t i = 0; i < TRAIN_COUNT; i++) {
        train_init(&singleton_trains[i], train_index_to_id(i));
    }

    last_read_sensor_time = 0;
    last_print_status_time = 0;
}

static void ts_try_read_sensors(TrainSensorList *sensorlist) {
    uint32_t now = (uint32_t) timer_read(TIMER3);
    if (now - last_read_sensor_time > READ_SENSOR_INTERVAL) {
        last_read_sensor_time = now;
        controller_read_sensors(sensorlist);
    }
}

static void ts_try_print_status(int iotid, TrainSensorList *sensorlist) {
    uint32_t now = (uint32_t) timer_read(TIMER3);
    if (now - last_print_status_time > PRINT_STATUS_INTERVAL) {
        last_print_status_time = now;
        for (uint32_t i = 0; i < sensorlist->size; i++) {
            PrintSensor(iotid, &sensorlist->sensors[i], now);
        }
        for (uint32_t i = 0; i < TRAIN_COUNT; i++) {
            PrintLocation(iotid, &singleton_trains[i]);
        }
    }
}

static void train_root_task() {
    int tid;
    TrainRequest request;
    TrainSensorList sensorlist;

    RegisterAs(SERVER_NAME_TRAIN);
    int iotid = WhoIs(SERVER_NAME_IO);
    train_server_init();
    controller_init(iotid);
    controller_go(0);
    controller_speed_all(0, 0);
    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        if (request.type == TRAIN_REQUEST_INIT_TRACK) {
            TrackName name = (TrackName) request.args[0];
            track_init(&singleton_track, name);
            controller_switch_all(DIR_CURVED, 0);
            controller_switch_one(6,  DIR_STRAIGHT, 0);
            controller_switch_one(9,  DIR_STRAIGHT, 0);
            controller_switch_one(15, DIR_STRAIGHT, 0);
        }
        if (request.type == TRAIN_REQUEST_INIT_TRAIN) {
            uint32_t train_id = request.args[0];
            TrackNode *node   = (TrackNode *) request.args[1];
            Train *train = train_find(singleton_trains, train_id);
            train->inited = true;
            train->position.node   = node;
            train->position.offset = 0;
        }
        if (request.type == TRAIN_REQUEST_WAKE_CONTROLLER) {
            controller_wake();
        }
        if (request.type == TRAIN_REQUEST_LOCATE_TRAINS) {
            for (int i = 0; i < TRAIN_COUNT; i++) {
                model_estimate_train_status(&singleton_trains[i]);
            }
            sensorlist.size = 0;
            ts_try_read_sensors(&sensorlist);
            ts_try_print_status(iotid, &sensorlist);
            model_correct_train_status(&sensorlist);
            train_manager_issue_directives();
        }
        if (request.type == TRAIN_REQUEST_SPEED) {
            uint32_t train_id = request.args[0];
            uint32_t speed    = request.args[1];
            controller_speed_one(train_id, speed, 0);
        }
        if (request.type == TRAIN_REQUEST_REVERSE) {
            uint32_t train_id = request.args[0];
            Train *train = train_find(singleton_trains, train_id);
            train->reverse = true;
            train->original_speed = train->speed;
            controller_speed_one(train_id, 0, 0);
        }
        if (request.type == TRAIN_REQUEST_MOVE) {
            uint32_t train_id = request.args[0];
            uint32_t speed    = request.args[1];
            TrackNode *sensor = (TrackNode *) request.args[2];
            int32_t offset    = (int32_t)     request.args[3];
            train_manager_navigate_train(train_id, sensor, offset);
            controller_speed_one(train_id, speed, 0);
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
        if (i % 10 == 0) {
            assert(Send(traintid, (char *)&locate_request, sizeof(locate_request), NULL, 0) >= 0);
        }
    }
    Exit();
}

void CreateTrainServer() {
    Create(PRIORITY_SERVER_TRAIN, &train_root_task);
    Create(PRIORITY_NOTIFIER_TRAIN, &train_notifier_task);
}
