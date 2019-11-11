#ifndef __TRAIN_TRAINS_H__
#define __TRAIN_TRAINS_H__

#include <stdint.h>
#include <train/track.h>

#define TRAIN_COUNT 6

typedef struct {
    uint32_t id;
    int speed;
    TrackPosition position;
    uint64_t last_position_update_time;
    uint64_t next_sensor_expected_time;

    int        omit_flag;
    uint64_t   prev_touch_time;
    TrackNode *prev_touch_node;
} Train;

int train_id_to_index(uint32_t train_id);

void trains_init();

Train *trains_iterate(int index);

Train *trains_find(uint32_t train_id);

void trains_set_speed(uint32_t train_id, int speed);

void trains_set_position(uint32_t train_id, TrackNode *node, uint32_t offset);

void trains_estimite_position(int index);

void trains_touch_sensor(uint32_t train_id, TrackNode *sensor);

#endif /*__TRAIN_TRAINS_H__*/