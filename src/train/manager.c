#include <hardware/timer.h>
#include <train/manager.h>
#include <train/model.h>
#include <train/train.h>
#include <user/ui.h>
#include <utils/assert.h>
#include <utils/queue.h>

#define TRAILLING_DISTANCE      200
#define PREPARE_AHEAD_DISTANCE  600

extern Track singleton_track;
extern Train singleton_trains[TRAIN_COUNT];

void train_manager_navigate_train(Train *train, TrackNode *dest, int32_t offset) {
    if (!singleton_track.inited || !train->inited) return;

    TrackPosition destination = position_move((TrackPosition) {dest, 0}, offset);
    if (destination.node == NULL) return;
    TrackPath path = search_path_to_node(&singleton_track, train->position.node, destination.node);
    if (path.dist == 0) return;

    train->routing = true;
    train->path = path;
    train->reverse_position.node = NULL;
    train->reverse_position.offset = 0;
    train->final_position.node = path_end(&train->path);
    train->final_position.offset = 0;
}

static bool train_manager_will_collide(Train *train, Train *other) {
    TrackPosition detect_range_start, detect_range_end;
    // Case 1: train and other are in the same direction
    detect_range_start = train->position;
    detect_range_end   = position_move(train->position, (int32_t) (train->stop_distance + TRAILLING_DISTANCE));
    if (position_in_range(other->position, detect_range_start, detect_range_end)) {
        return true;
    }
    // Case 2: train and other are in the opposite direction
    detect_range_start = train->position;
    detect_range_end   = position_move(train->position, (int32_t) (train->stop_distance + other->stop_distance + TRAILLING_DISTANCE));
    assert(other->position.node != NULL);
    if (position_in_range(position_reverse(other->position), detect_range_start, detect_range_end)) {
        return true;
    }
    return false;
}

bool train_manager_unblocked_train(Train *train) {
    (void)train;
    return true;
}

bool train_manager_will_collide_train(Train *train) {
    for (size_t i = 0; i < TRAIN_COUNT; i++) {
        Train *other = &singleton_trains[i];
        if (other->inited && other->id != train->id && train_manager_will_collide(train, other)) {
            return true;
        }
    }
    return false;
}

static bool train_manager_will_arrive_position(Train *train, TrackPosition *position) {
    TrackPosition stop_range_start = train->position;
    TrackPosition stop_range_end   = position_move(train->position, (int32_t) (train->stop_distance));
    return position_in_range(*position, stop_range_start, stop_range_end);
}

bool train_manager_will_arrive_final(Train *train) {
    return (train->final_position.node != NULL) &&
        train_manager_will_arrive_position(train, &train->final_position);
}

bool train_manager_will_arrive_reverse(Train *train) {
    return (train->reverse_position.node != NULL) &&
        train_manager_will_arrive_position(train, &train->reverse_position);
}

static void train_manager_reserve_branch(uint32_t train_id, TrackNode *node) {
    assert(node->owner == node->reverse->owner);
    if (train_id < node->owner) {
        node->owner = train_id;
        node->reverse->owner = train_id;
    }
}

static void train_manager_reserve_branches(Train *train) {
    // Reserve the branches around the train.
    TrackPosition around_start = position_move(train->position, -400);
    TrackPosition around_end   = position_move(train->position,  400);
    assert(around_start.node != NULL && around_end.node != NULL);
    for (size_t i = 0; i < singleton_track.node_count; i++) {
        TrackNode *node = &singleton_track.nodes[i];
        if (node->type == NODE_BRANCH || node->type == NODE_MERGE) {
            if (position_in_range((TrackPosition) { node, 0 }, around_start, around_end)) {
                train_manager_reserve_branch(train->id, node);
            }
        }
    }
    // Reserve the branches on the coming path.
    TrackPath subpath = path_to_greater_length(&train->path, train->stop_distance + PREPARE_AHEAD_DISTANCE + 200);
    for (uint32_t i = 0; i < subpath.list.size; i++) {
        TrackEdge *edge = edgelist_by_index(&subpath.list, i);
        TrackNode *src = edge->src;
        assert(edge != NULL && src != NULL);
        if (src->type == NODE_BRANCH || src->type == NODE_MERGE) {
            train_manager_reserve_branch(train->id, src);
        }
    }
}

static void train_manager_release_branches(Train *train) {
    for (size_t i = 0; i < singleton_track.node_count; i++) {
        if (singleton_track.nodes[i].owner == train->id) {
            singleton_track.nodes[i].owner = UINT32_MAX;
        }
    }
}

static void train_manager_prepare_ahead(Train *train) {
    TrackPath subpath = path_to_greater_length(&train->path, train->stop_distance + PREPARE_AHEAD_DISTANCE);
    for (uint32_t i = 0; i < subpath.list.size; i++) {
        TrackEdge *edge = edgelist_by_index(&subpath.list, i);
        TrackNode *src = edge->src;
        assert(edge != NULL && src != NULL);
        if (src->type == NODE_BRANCH && edge_direction(edge) != src->direction) {
            if (src->owner == train->id) {
                controller_switch_one(src->num, edge_direction(edge), 0);
            }
            /*} else {*/
                /*controller_speed_one(train->id, 0, 0);*/
            /*}*/
        }
        /*if (src->type == NODE_MERGE) {*/
            /*if (src->owner != train->id) {*/
                /*controller_speed_one(train->id, 0, 0);*/
            /*}*/
        /*}*/
        /*if (!train->reverse && edge_direction(edge) == DIR_REVERSE) {*/
            /*train->reverse = true;*/
            /*train->original_speed = train->speed;*/
            /*train->stop_position.node = edge->src;*/
            /*train->stop_position.offset = 0;*/
            /*train->stop_position = position_move(train->stop_position, 50);*/
        /*}*/
    }
}

static void train_manager_update_routing(Train *train) {
    path_rebase(&train->path, train->position.node);
    train_manager_prepare_ahead(train);
}

void train_manager_issue_directives() {
    if (!singleton_track.inited) return;
    for (size_t i = 0; i < TRAIN_COUNT; i++) {
        Train *train = &singleton_trains[i];
        if (!train->inited) continue;

        train_manager_release_branches(train);
        train_manager_reserve_branches(train);
        if (train->routing) train_manager_update_routing(train);
        train->driver_handle(train);
    }
}
