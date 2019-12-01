#ifndef __TRAIN_TRAIN_H__
#define __TRAIN_TRAIN_H__

#include <stdint.h>
#include <train/track.h>

#define TRAIN_COUNT 6

typedef enum {
    TRAIN_STATE_NONE,
    TRAIN_STATE_CRUISE,
    TRAIN_STATE_BRAKE_COMMAND,
    TRAIN_STATE_BRAKE_REVERSE,
    TRAIN_STATE_BRAKE_TRAFFIC,
    TRAIN_STATE_WAIT_COMMAND,
    TRAIN_STATE_WAIT_REVERSE,
    TRAIN_STATE_WAIT_TRAFFIC,
} TrainState;

typedef enum {
    TRAIN_MODE_FREE,
    TRAIN_MODE_PATH,
    TRAIN_MODE_ROAM,
} TrainMode;

typedef struct Train Train;

struct Train {
    bool inited;
    uint32_t id;
    uint32_t speed;
    uint32_t original_speed;

    uint32_t velocity;
    uint32_t stop_distance;
    TrackPosition position;
    uint32_t model_last_update_time;

    TrainState state;
    void (*driver_handle)(Train *);

    TrainMode mode;
    TrackPath path;
    TrackPosition reverse_position;
    TrackPosition final_position;
};

/**
 * Initialize train with id
 */
void train_init(Train *train, uint32_t id);

/**
 * Reset the variables to default values
 */
void train_clear(Train *train);

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
