#include <train/trains.h>
#include <hardware/timer.h>
#include <utils/assert.h>

static const uint32_t train_ids[TRAIN_COUNT] = { 1, 24, 58, 74, 78, 79 };
Train trains[TRAIN_COUNT];

static uint32_t velocities[15] = {
    0, 40, 80, 120, 160, 200, 260, 320, 370, 430, 465, 530, 560, 0, 0,
};

int train_id_to_index(uint32_t train_id) {
    if (train_id ==  1) return 0;
    if (train_id == 24) return 1;
    if (train_id == 58) return 2;
    if (train_id == 74) return 3;
    if (train_id == 78) return 4;
    if (train_id == 79) return 5;
    throw("unknown train id");
}

uint32_t train_speed_to_velocity(int speed) {
    assert(speed >= 0 && speed <= 14);
    return (uint32_t) speed * 40;
}

uint32_t train_expected_time(int speed, uint32_t mm) {
    return (mm * 1000) / train_speed_to_velocity(speed);
}

uint32_t train_expected_distance(int speed, uint32_t ms) {
    return (train_speed_to_velocity(speed) * ms) / 1000;
}

void trains_init() {
    for (size_t i = 0; i < TRAIN_COUNT; i++) {
        trains[i].id = train_ids[i];
        trains[i].speed = 0;
        trains[i].position.node = NULL;
        trains[i].position.dist = 0;
        trains[i].last_position_update_time = 0;
        trains[i].next_sensor_expected_time = 0;

        trains[i].omit_flag = 0;
    }
}

Train *trains_iterate(int index) {
    return &(trains[index]);
}

Train *trains_find(uint32_t train_id) {
    return trains_iterate(train_id_to_index(train_id));
}

static void _trains_set_speed(int index, int speed) {
    trains[index].speed = speed;
}

void trains_set_speed(uint32_t train_id, int speed) {
    int index = train_id_to_index(train_id);
    _trains_set_speed(index, speed);
}

static void _trains_set_position(int index, TrackNode *node, uint32_t offset) {
    trains[index].position.node = node;
    trains[index].position.dist = offset;
    trains[index].last_position_update_time = timer_read(TIMER3);
}

void trains_set_position(uint32_t train_id, TrackNode *node, uint32_t offset) {
    int index = train_id_to_index(train_id);
    _trains_set_position(index, node, offset);
}

void trains_estimite_position(int index) {
    if (trains[index].position.node != NULL) {
        uint64_t now = timer_read(TIMER3);
        uint64_t escaped = now - trains[index].last_position_update_time;
        uint32_t dist = train_expected_distance(trains[index].speed, escaped);
        track_position_move(&trains[index].position, dist);
        trains[index].last_position_update_time = now;
    }
}

void trains_touch_sensor(uint32_t train_id, TrackNode *sensor) {
    assert(sensor != NULL);
    int index = train_id_to_index(train_id);
    _trains_set_position(index, sensor, 0);
    uint32_t dist = track_find_next_sensor_dist(sensor);
    if (dist != (uint32_t) -1) {
        uint64_t expected_time = train_expected_time(trains[index].speed, dist);
        trains[index].next_sensor_expected_time = timer_read(TIMER3) + expected_time;
    }

    if (trains[index].omit_flag) {
        trains[index].omit_flag = 0;
    } else {
        trains[index].prev_touch_time = timer_read(TIMER3);
        trains[index].prev_touch_node = sensor;
        trains[index].omit_flag = 1;
    }
}
