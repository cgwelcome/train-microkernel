#ifndef __TRAIN_TRAIN_H__
#define __TRAIN_TRAIN_H__

#include <stdint.h>
#include <train/track.h>

#define TRAIN_COUNT 6

typedef struct {
    TrackNode *node;
    uint64_t expected_time;
    uint64_t actual_time;
} Checkpoint;

typedef struct {
    bool inited;
    uint32_t id;
    uint32_t speed;
    uint32_t velocity;

    TrackPosition position;
    uint32_t last_position_update_time;

    Checkpoint last_checkpoint;
    Checkpoint next_checkpoint;
    TrackPath path;
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
 * If the train is close to a position (within 50mm), returns the distance
 * between the train and the position. Otherwise, returns UINT32_MAX.
 */
uint32_t train_close_to(Train *train, TrackPosition dest);

#endif /*__TRAIN_TRAIN_H__*/
