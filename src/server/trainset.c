#include <priority.h>
#include <server/io.h>
#include <server/clock.h>
#include <server/trainset.h>
#include <user/clock.h>
#include <user/io.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>
#include <utils/bwio.h>
#include <utils/tsqueue.h>

// Trainset Server variables
static const uint32_t active_trains[] = {
	1, 24, 58, 74, 78, 79,
};
static uint32_t active_trains_size = sizeof(active_trains)/sizeof(active_trains[0]);
static int iotid;
static int uart;
static TSQueue delayresponses;
static Train trains[MAX_TRAIN_NUM];
static TrainSwitch trainswitches[MAX_SWITCH_NUM];

void delay_notifier_task() {
    TSTimerResponse response;
    TSRequest request = {
        .type = TSREQUESTTYPE_INIT_TIMEOUT,
    };
    int servertid = WhoIs(TRAINSET_SERVER_NAME);
    int clocktid = WhoIs(CLOCK_SERVER_NAME);

    Send(servertid, (char *)&request, sizeof(request), (char *)&response, sizeof(response));
    Delay(clocktid, (int)response.delay/10);
    Send(servertid, (char *)&response.payload, sizeof(response.payload), NULL, 0);
    Exit();
}

static void trainset_create_delay(TSRequest payload, uint32_t delay) {
    TSTimerResponse response = {
        .payload = payload,
        .delay = delay
    };
    tsqueue_push(&delayresponses, response);
    Create(2900, &delay_notifier_task);
}

static void trainset_go() {
    Putc(iotid, uart, TRAINSET_GO);
}

static void trainset_stop() {
    Putc(iotid, uart, TRAINSET_STOP);
}

static void trainset_speed(Train *train, uint32_t speed) {
    Putc(iotid, uart, (char)speed);
    Putc(iotid, uart, (char)train->id);
    trains[train->id].speed = speed;
}

static void trainset_reverse(Train *train) {
    // Save old speed, after trainset_speed(), train->speed == 0
    uint32_t speed = train->speed;
    trainset_speed(train, 0);
    TSRequest payload = {
        .type = TSREQUESTTYPE_REVERSE_TIMEOUT,
        .arg1 = train->id,
        .arg2 = speed,
    };
    trainset_create_delay(payload, TRAIN_REVERSE_INTERVAL);
}

static void trainset_switch(TrainSwitch *trainswitch, TrainSwitchStatus status) {
    switch (status) {
        case SWITCHSTATUS_STRAIGHT:
            Putc(iotid, uart, SWITCH_STRAIGHT);
            break;
        case SWITCHSTATUS_CURVED:
            Putc(iotid, uart, SWITCH_CURVED);
            break;
        default:
            throw("unknown switch status");
    }
    Putc(iotid, uart, (char)trainswitch->id);
    trainswitch->status = status;
}

static void trainset_switchone(TrainSwitch *trainswitch, TrainSwitchStatus status) {
    trainset_switch(trainswitch, status);
    TSRequest payload = {
        .type = TSREQUESTTYPE_SWITCH_TIMEOUT,
    };
    trainset_create_delay(payload, SWITCH_MOVE_INTERVAL);
}

static void trainset_switchall(TrainSwitchStatus status) {
    for (uint32_t id = 1; id <= 18; id++) {
        trainswitches[id].id = id;
        trainset_switch(&trainswitches[id], status);
    }
    for (uint32_t id = 0x99; id <= 0x9C; id++) {
        trainswitches[id].id = id;
        trainset_switch(&trainswitches[id], status);
    }
    TSRequest payload = {
        .type = TSREQUESTTYPE_SWITCH_TIMEOUT,
    };
    trainset_create_delay(payload, SWITCH_MOVE_INTERVAL);
}

static void trainset_sensor_parsemodule(ActiveTrainSensorList *sensorlist, char module, uint16_t raw) {
    for (uint32_t i = 0; i < MAX_SENSOR_PER_MODULE; i++) {
        if (raw % 2 == 1) {
            TrainSensor sensor = {
                .id = MAX_SENSOR_PER_MODULE - i,
                .module = module,
            };
            sensorlist->sensors[sensorlist->size] = sensor;
            sensorlist->size++;
        }
        raw = raw >> 1;
    }
}

static void trainset_sensor_readall(int tid) {
    ActiveTrainSensorList sensorlist;
    sensorlist.size = 0;
    Putc(iotid, uart, (char)(SENSOR_READALL + MODULE_TOTAL_NUM));
    for (char module = 'A'; module < 'A' + MODULE_TOTAL_NUM; module++) {
        uint8_t first = (uint8_t) Getc(iotid, uart);
        uint8_t second = (uint8_t) Getc(iotid, uart);
        uint16_t raw = (uint16_t)((first << sizeof(second)*8) | second);
        trainset_sensor_parsemodule(&sensorlist, module, raw);
    }
    Reply(tid, (char *)&sensorlist, sizeof(sensorlist));
}

static void trainset_init_timeout(int notifier_tid) {
    TSTimerResponse response = tsqueue_pop(&delayresponses);
    Reply(notifier_tid, (char *)&response, sizeof(response));
}

static void trainset_reverse_timeout(Train *train, uint32_t speed) {
    trainset_speed(train, TRAIN_REVERSE);
    trainset_speed(train, speed);
}

static void trainset_switch_timeout() {
    Putc(iotid, uart, (char)SWITCH_DONE);
}

static void trainset_init() {
    uart = COM1;
    iotid = WhoIs(IO_SERVER_NAME);
    RegisterAs(TRAINSET_SERVER_NAME);
    tsqueue_init(&delayresponses);
    for (uint32_t i = 0; i < active_trains_size; i++) {
		uint32_t id = active_trains[i];
        trains[id].id = id;
        trainset_speed(&trains[id], 0);
    }
    trainset_go();
    trainset_switchall(SWITCHSTATUS_DEFAULT);
}

static void trainset_done() {
	for (uint32_t i = 0; i < active_trains_size; i++) {
		uint32_t id = active_trains[i];
		trainset_speed(&trains[id], 0);
	}
}

void trainset_server_task() {
    int tid;
    TSRequest request;
    trainset_init();

    for (;;) {
        Receive(&tid, (char *)&request, sizeof(request));
        switch (request.type) {
            case TSREQUESTTYPE_GO:
                trainset_go();
                Reply(tid, NULL, 0);
                break;
            case TSREQUESTTYPE_STOP:
                trainset_stop();
                Reply(tid, NULL, 0);
                break;
            case TSREQUESTTYPE_SPEED:
                trainset_speed(&trains[request.arg1], request.arg2);
                Reply(tid, NULL, 0);
                break;
            case TSREQUESTTYPE_REVERSE:
                trainset_reverse(&trains[request.arg1]);
                Reply(tid, NULL, 0);
                break;
            case TSREQUESTTYPE_SWITCHONE:
                trainset_switchone(&trainswitches[request.arg1], request.arg2);
                Reply(tid, NULL, 0);
                break;
            case TSREQUESTTYPE_SWITCHALL:
                trainset_switchall(request.arg1);
                Reply(tid, NULL, 0);
                break;
            case TSREQUESTTYPE_SENSOR_READALL:
                trainset_sensor_readall(tid);
                break;
            case TSREQUESTTYPE_INIT_TIMEOUT:
                trainset_init_timeout(tid);
                break;
            case TSREQUESTTYPE_REVERSE_TIMEOUT:
                trainset_reverse_timeout(&trains[request.arg1], request.arg2);
                Reply(tid, NULL, 0);
                break;
            case TSREQUESTTYPE_SWITCH_TIMEOUT:
                trainset_switch_timeout();
                Reply(tid, NULL, 0);
                break;
            case TSREQUESTTYPE_DONE:
                trainset_done();
                Reply(tid, NULL, 0);
                break;
            default:
                throw("unknown request");
        }
    }
    Exit();
}


int CreateTrainSetServer() {
    return Create(PRIORITY_SERVER_TRAINSET, &trainset_server_task);
}
