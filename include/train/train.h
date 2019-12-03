#ifndef __TRAIN_TRAIN_H__
#define __TRAIN_TRAIN_H__

#include <stdint.h>
#include <train/track.h>

#define TRAIN_COUNT 6

typedef enum {
    TRAIN_DIRECTION_FORWARD,
    TRAIN_DIRECTION_BACKWARD,
} TrainDirection;

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

    // Physical model
    TrainDirection direction;
    uint32_t velocity;
    uint32_t stop_distance;
    TrackPosition position;
    uint32_t model_last_update_time;

    // Driver DFA
    TrainState state;
    void (*driver_handle)(Train *);

    // Navigation
    TrainMode mode;
    TrackPath path;
    TrackPosition reverse_anchor;
    TrackPosition reverse_position;
    TrackPath reverse_path;
    TrackPosition final_position;

    // Traffic
    Train *blocked_train;
    TrackNode *blocked_switch;

    // Error detection
    uint32_t missing_count;
    TrackNode *missing_sensor;
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
 * Reverse the position of the train.
 */
void train_reverse_position(Train *train);

/**
 * If the train is close to a position (within tolerance in mm),
 * returns the distance between the train and the position.
 * Otherwise, returns UINT32_MAX.
 */
uint32_t train_close_to(Train *train, TrackPosition dest, int32_t tolerance);

/**
 * Notify that the train has touched a sensor.
 */
void train_touch_sensor(Train *train, TrackNode* sensor);

#endif /*__TRAIN_TRAIN_H__*/
