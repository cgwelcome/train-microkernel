#include <hardware/timer.h>
#include <train/manager.h>
#include <train/model.h>
#include <train/train.h>
#include <user/ui.h>
#include <utils/assert.h>
#include <utils/queue.h>

#define TRAILLING_DISTANCE 250
#define SWITCH_RESERVATION_DISTANCE 1000

extern Track singleton_track;
extern Train singleton_trains[TRAIN_COUNT];

void train_manager_navigate_train(uint32_t train_id, TrackNode *dest, int32_t offset) {
    Train *train = train_find(singleton_trains, train_id);
    if (!singleton_track.inited || !train->inited) return;

    TrackPosition destination = position_standardize(dest, offset);
    if (destination.node == NULL) return;
    TrackPath path = search_path_to_node(&singleton_track, train->position.node, destination.node);
    if (path.dist == 0) return;

    train->trajectory = true;
    train->path = path;
    train->destination = destination;
    train->stop_destination = destination;
}

static void train_manager_prepare_ahead(Train *train) {
    TrackPath subpath = path_to_greater_length(&train->path, SWITCH_RESERVATION_DISTANCE);
    for (uint32_t i = 0; i < subpath.list.size; i++) {
        TrackEdge *edge = edgelist_by_index(&subpath.list, i);
        TrackNode *src = edge->src;
        if (src->type == NODE_BRANCH && edge_direction(edge) != src->direction) {
            controller_switch_one(src->num, edge_direction(edge), 0);
        }
        if (!train->reverse && edge_direction(edge) == DIR_REVERSE) {
            // Prepare Stop destination
            train->reverse = true;
            train->original_speed = train->speed;
            train->stop_destination.node = edge->dest;
            train->stop_destination.offset = 100;
        }
    }
}

static bool train_manager_will_arrive(Train *train) {
    TrackPosition stop_range_start = train->position;
    TrackPosition stop_range_end   = position_move(train->position, (int32_t) (train->stop_distance));
    return position_in_range(train->stop_destination, stop_range_start, stop_range_end);
}

static bool train_manager_will_collide(Train *train, Train *other) {
    TrackPosition detect_range_start = train->position;
    TrackPosition detect_range_end   = position_move(train->position, (int32_t) (train->stop_distance + TRAILLING_DISTANCE));
    return position_in_range(other->position, detect_range_start, detect_range_end);
}

static void train_manager_look_ahead(Train *train) {
    for (size_t i = 0; i < TRAIN_COUNT; i++) {
        Train *other = &singleton_trains[i];
        if (other->inited && other->id != train->id) {
            if (train_manager_will_collide(train, other)) {
                controller_speed_one(train->id, 0, 0);
            }
        }
    }
}

static void train_manager_handle_reverse(Train *train) {
    train->reverse = false;
    controller_speed_one(train->id, TRAIN_STATUS_REVERSE, 0);
    controller_speed_one(train->id, train->original_speed, 0);
    train->position = position_reverse(train->position);
    if (train->trajectory) {
        train->stop_destination = train->destination;
    }
}

void train_manager_issue_directives() {
    for (size_t i = 0; i < TRAIN_COUNT; i++) {
        Train *train = &singleton_trains[i];
        if (train->inited) {
            if (train->reverse && train->velocity == 0) {
                train_manager_handle_reverse(train);
            }
            if (train->trajectory) {
                path_move(&train->path, train->position.node);
                train_manager_prepare_ahead(train);
            }
            if (train->stop_destination.node != NULL && train_manager_will_arrive(train)) {
                controller_speed_one(train->id, 0, 0);
                train->stop_destination.node = NULL;
            }
            train_manager_look_ahead(train);
        }
    }
}
