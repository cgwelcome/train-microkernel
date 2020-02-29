#include <kernel.h>
#include <hardware/timer.h>
#include <train/controller.h>
#include <train/track.h>
#include <train/train.h>
#include <user/io.h>
#include <user/name.h>
#include <user/ui.h>
#include <user/name.h>
#include <utils/assert.h>
#include <utils/pqueue.h>

extern Track singleton_track;
extern Train singleton_trains[TRAIN_COUNT];

static int iotid;
static uint32_t light_switch;
static uint32_t last_switch_time;
static PQueue directive_queue;
static TrainDirective directives[CONTROLLER_DIRECTIVE_LIMIT];

void controller_init(int tid) {
    iotid = tid;
    light_switch = 0;
    last_switch_time = (uint32_t) -1;
    pqueue_init(&directive_queue);
    for (int i = 0; i < CONTROLLER_DIRECTIVE_LIMIT; i++) {
        directives[i].type = TRAIN_DIRECTIVE_NONE;
    }
}

static int controller_schedule_next_directive(TrainDirective *directive) {
    for (int i = 0; i < CONTROLLER_DIRECTIVE_LIMIT; i++) {
        if (directives[i].type == TRAIN_DIRECTIVE_NONE) {
            directives[i] = *directive;
            return i;
        }
    }
    throw("out of space for new directive");
}

static void controller_handle_directive(TrainDirective *directive) {
    switch (directive->type) {
        case TRAIN_DIRECTIVE_SPEED:
            Printf(iotid, COM1, "%c%c", (char) (directive->data + light_switch), (char) directive->id);
            if (directive->data != TRAIN_STATUS_REVERSE) {
                train_find(singleton_trains, directive->id)->speed = directive->data;
            }
            break;
        case TRAIN_DIRECTIVE_SWITCH:
            switch (directive->data) {
                case DIR_STRAIGHT:
                    Printf(iotid, COM1, "%c%c", (char)TRAIN_CODE_SWITCH_STRAIGHT, (char)directive->id);
                    break;
                case DIR_CURVED:
                    Printf(iotid, COM1, "%c%c", (char)TRAIN_CODE_SWITCH_CURVED  , (char)directive->id);
                    break;
                default:
                    throw("unknow switch status");
            }
            last_switch_time = (uint32_t) timer_read(TIMER3);
            if (singleton_track.inited) {
                TrackNode *branch = track_find_branch(&singleton_track, directive->id);
                if (!branch->broken) {
                    branch->direction = (uint8_t) directive->data;
                }
                PrintSwitch(iotid, branch->num, branch->direction);
            }
            break;
        default:
            throw("unacceptable directive type");
    }
}

void controller_wake() {
    uint32_t now = (uint32_t) timer_read(TIMER3);
    while (pqueue_size(&directive_queue) > 0 && (uint32_t) pqueue_peek(&directive_queue) <= now) {
        int index = pqueue_pop(&directive_queue);
        controller_handle_directive(&directives[index]);
        directives[index].type = TRAIN_DIRECTIVE_NONE;
    }
}

static void controller_schedule(TrainDirectiveType type, uint32_t id, uint32_t data, uint32_t delay /*in ms*/) {
    uint32_t now = (uint32_t) timer_read(TIMER3);
    if (last_switch_time != (uint32_t) -1) {
        if (now - last_switch_time >= CONTROLLER_SWITCH_DONE_TIMEOUT) {
            Putc(iotid, COM1, TRAIN_CODE_SWITCH_DONE);
            last_switch_time = (uint32_t) -1;
        }
    }
    TrainDirective directive = { .type = type, .id = id, .data = data };
    if (delay == 0) {
        controller_handle_directive(&directive);
    } else {
        int index = controller_schedule_next_directive(&directive);
        pqueue_insert(&directive_queue, index, (int32_t)(now + delay));
    }
}

void controller_go() {
    Putc(iotid, COM1, TRAIN_CODE_GO);
}

void controller_stop() {
    Putc(iotid, COM1, TRAIN_CODE_STOP);
}

void controller_set_light(bool turn_on) {
    if (turn_on) {
        light_switch = TRAIN_STATUS_LIGHT;
    } else {
        light_switch = 0;
    }
}

void controller_speed_one(uint32_t train_id, uint32_t speed, uint32_t delay) {
    controller_schedule(TRAIN_DIRECTIVE_SPEED, train_id, speed, delay);
}

void controller_speed_all(uint32_t speed, uint32_t delay) {
    for (int i = 0; i < TRAIN_COUNT; i++) {
        controller_speed_one(singleton_trains[i].id, speed, delay);
    }
}

void controller_switch_one(uint32_t switch_id, uint32_t direction, uint32_t delay) {
    controller_schedule(TRAIN_DIRECTIVE_SWITCH, switch_id, direction, delay);
}

void controller_switch_all(uint32_t direction, uint32_t delay) {
    for (uint32_t id = 1; id <= 18; id++) {
        controller_schedule(TRAIN_DIRECTIVE_SWITCH, id, direction, delay);
    }
    for (uint32_t id = 0x99; id <= 0x9C; id++) {
        controller_schedule(TRAIN_DIRECTIVE_SWITCH, id, direction, delay);
    }
}

static void controller_parse_sensor(TrainSensorList *sensorlist, char module, uint16_t raw) {
    for (uint32_t i = 0; i < MAX_SENSOR_PER_MODULE; i++) {
        if (raw % 2 == 1) {
            sensorlist->sensors[sensorlist->size].module = module;
            sensorlist->sensors[sensorlist->size].id = MAX_SENSOR_PER_MODULE - i;
            sensorlist->size++;
        }
        raw = raw >> 1;
    }
}

void controller_read_sensors(TrainSensorList *sensorlist) {
    sensorlist->size = 0;
    Putc(iotid, COM1, (char)(TRAIN_CODE_SENSOR_ALL + MODULE_TOTAL_NUM));
    for (char module = 'A'; module < 'A' + MODULE_TOTAL_NUM; module++) {
        uint16_t raw = 0; char *addr = (char *) &raw;
        Getc(iotid, COM1, addr + 1);
        Getc(iotid, COM1, addr + 0);
        controller_parse_sensor(sensorlist, module, raw);
    }
}
