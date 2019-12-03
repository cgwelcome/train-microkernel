#include <train/train.h>
#include <hardware/timer.h>
#include <utils/assert.h>

Train singleton_trains[TRAIN_COUNT];

const uint32_t train_ids[TRAIN_COUNT] = { 1, 24, 58, 74, 78, 79 };

void train_init(Train *train, uint32_t id) {
    train->id = id;
    train_clear(train);
}

void train_clear(Train *train) {
    train->inited = false;
    train->speed = 0;
    train->original_speed = 0;

    train->direction = TRAIN_DIRECTION_FORWARD;
    train->velocity = 0;
    train->stop_distance = 0;
    position_clear(&train->position);
    train->model_last_update_time = 0;

    train->state = TRAIN_STATE_NONE;
    train->driver_handle = NULL;

    train->mode = TRAIN_MODE_FREE;
    path_clear(&train->path);
    position_clear(&train->reverse_anchor);
    position_clear(&train->reverse_position);
    path_clear(&train->reverse_path);
    position_clear(&train->final_position);

    train->blocked_train = NULL;
    train->blocked_switch = NULL;

    train->miss_count = 0;
    train->recent_sensor = NULL;
    train->missed_sensor = NULL;
}

uint32_t train_id_to_index(uint32_t train_id) {
    if (train_id ==  1) return 0;
    if (train_id == 24) return 1;
    if (train_id == 58) return 2;
    if (train_id == 74) return 3;
    if (train_id == 78) return 4;
    if (train_id == 79) return 5;
    throw("unknown train id %u", train_id);
}

uint32_t train_index_to_id(uint32_t index) {
    return train_ids[index];
}

Train *train_find(Train *trains, uint32_t train_id) {
    return &trains[train_id_to_index(train_id)];
}

void train_move_forward(Train *train, uint32_t offset) {
    assert(train->position.edge != NULL);

    TrackEdge *last_edge = train->position.edge;
    train->position = position_move(train->position, (int32_t) offset);
    TrackEdge *curr_edge = train->position.edge;

    // Miss a sensor hit, record it for error handling
    if (last_edge != curr_edge && curr_edge->src->type == NODE_SENSOR) {
        if (train->recent_sensor != curr_edge->src && train->recent_sensor != curr_edge->src->reverse) {
            train->miss_count += 1;
            if (train->missed_sensor == NULL) {
                train->missed_sensor = curr_edge->src;
            }
        }
    }
}

void train_reverse_position(Train *train) {
    if (train->direction == TRAIN_DIRECTION_FORWARD) {
        train->direction = TRAIN_DIRECTION_BACKWARD;
    } else {
        train->direction = TRAIN_DIRECTION_FORWARD;
    }
    train->position = position_reverse(train->position);
}

uint32_t train_close_to(Train *train, TrackPosition dest, uint32_t tolerance) {
    assert(train->inited);

    if (dest.edge == NULL) {
        return UINT32_MAX;
    }
    uint32_t dist_front  = position_dist(train->position, dest, tolerance);
    uint32_t dist_behind = position_dist(position_reverse(train->position), dest, tolerance);
    return dist_front < dist_behind ? dist_front : dist_behind;
}

void train_touch_sensor(Train *train, TrackNode* sensor) {
    train->miss_count = 0;
    train->recent_sensor = sensor;
    train->missed_sensor = NULL;

    train->position = (TrackPosition) {
        .edge   = node_select_next_edge(sensor),
        .offset = 0,
    };
    if (train->direction == TRAIN_DIRECTION_FORWARD) {
        train->position = position_move(train->position, -50);
    } else {
        train->position = position_move(train->position,  50);
    }
}
