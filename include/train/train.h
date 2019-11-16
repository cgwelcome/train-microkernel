#ifndef __TRAIN_TRAIN_H__
#define __TRAIN_TRAIN_H__

#include <stdint.h>
#include <train/track.h>

#define TRAIN_COUNT 6

typedef struct {
    uint32_t id;
    uint32_t speed;
    TrackPosition position;
    uint64_t last_position_update_time;
    uint64_t next_sensor_expected_time;

    int        omit_flag;
    uint64_t   prev_touch_time;
    TrackNode *prev_touch_node;
} Train;

/**
 * Initialize train with id
 */
void train_init(Train *train, uint32_t id);

/**
 * Convert a trains id to an index
 */
uint32_t train_id_to_index(uint32_t train_id);

/**
 * Convert an index to an id
 */
uint32_t train_index_to_id(uint32_t index);

/**
 * Using train_id_to_index, find the train in trains
 */
Train *train_find(Train *trains, uint32_t train_id);

/**
 * Convert speed to a velocity
 */
uint32_t train_speed_to_velocity(uint32_t speed);

/**
 * Given distance in mm, estimate the time with static velocity
 */
uint32_t train_expected_time(uint32_t speed, uint32_t mm);

/**
 * Given distance in mm, estimate the time with static velocity
 */
uint32_t train_expected_distance(uint32_t speed, uint64_t ms);

void train_set_position(Train *train, TrackNode *node, uint32_t dist);

void train_estimate_position(Train *train, Track *track);

void train_touch_sensor(Train *train, Track *track, TrackNode *sensor);

#endif /*__TRAIN_TRAIN_H__*/
