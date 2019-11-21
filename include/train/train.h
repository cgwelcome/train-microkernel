#ifndef __TRAIN_TRAIN_H__
#define __TRAIN_TRAIN_H__

#include <stdint.h>
#include <train/track.h>

#define TRAIN_COUNT 6

typedef struct {
    bool inited;
    uint32_t id;
    uint32_t speed;

    uint32_t velocity;
    uint32_t stop_distance;
    TrackPosition position;
    uint32_t model_last_update_time;

    bool blocked;
    bool reverse;
    bool trajectory;
    uint32_t original_speed;
    TrackPath path;
    TrackPosition stop_position;
    TrackPosition final_destination;
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
 * If the train is close to a position (within tolerance in mm),
 * returns the distance between the train and the position.
 * Otherwise, returns UINT32_MAX.
 */
uint32_t train_close_to(Train *train, TrackPosition dest, int32_t tolerance);

#endif /*__TRAIN_TRAIN_H__*/
