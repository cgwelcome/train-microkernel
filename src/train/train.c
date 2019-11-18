#include <train/train.h>
#include <hardware/timer.h>
#include <utils/assert.h>

Train singleton_trains[TRAIN_COUNT];

const uint32_t train_ids[TRAIN_COUNT] = { 1, 24, 58, 74, 78, 79 };

static const uint32_t velocities[15] = {
    0, 40, 80, 120, 160, 200, 260, 320, 370, 430, 465, 530, 560, 0, 0,
};

void train_init(Train *train, uint32_t id) {
    train->id = id;
    train->speed = 0;
    train->position.node = NULL;
    train->position.offset= 0;
    train->last_position_update_time = 0;
    train->next_sensor_expected_time = 0; train->omit_flag = 0;
}

uint32_t train_id_to_index(uint32_t train_id) {
    if (train_id ==  1) return 0;
    if (train_id == 24) return 1;
    if (train_id == 58) return 2;
    if (train_id == 74) return 3;
    if (train_id == 78) return 4;
    if (train_id == 79) return 5;
    throw("unknown train id");
}

uint32_t train_index_to_id(uint32_t index) {
    return train_ids[index];
}

Train *train_find(Train *trains, uint32_t train_id) {
    return &trains[train_id_to_index(train_id)];
}

uint32_t train_speed_to_velocity(uint32_t speed) {
    assert(speed <= 14);
    return (uint32_t)(speed * 40);
}

uint32_t train_expected_time(uint32_t speed, uint32_t mm) {
    return (mm * 1000) / train_speed_to_velocity(speed);
}

uint32_t train_expected_distance(uint32_t speed, uint64_t ms) {
    return (uint32_t)(train_speed_to_velocity(speed) * ms / 1000);
}

void train_set_position(Train *train, TrackNode *node, uint32_t dist) {
    train->position.node = node;
    train->position.offset = dist;
    train->last_position_update_time = timer_read(TIMER3);
}

void train_estimate_position(Train *train) {
    assert(train->position.node != NULL);
    if (train->position.node != NULL) {
        uint64_t now = timer_read(TIMER3);
        uint64_t escaped = now - train->last_position_update_time;
        uint32_t dist = train_expected_distance(train->speed, escaped);
        position_move(&train->position, (int32_t)dist);
        train->last_position_update_time = now;
    }
}

void train_touch_sensor(Train *train, TrackNode *sensor) {
    assert(sensor != NULL);
    train_set_position(train, sensor, 0);
    uint32_t dist = node_select_next_edge(sensor)->dist;
    if (dist != (uint32_t) -1) {
        uint64_t expected_time = train_expected_time(train->speed, dist);
        train->next_sensor_expected_time = timer_read(TIMER3) + expected_time;
    }
    if (train->omit_flag) {
        train->omit_flag = 0;
    } else {
        train->prev_touch_time = timer_read(TIMER3);
        train->prev_touch_node = sensor;
        train->omit_flag = 1;
    }
}
