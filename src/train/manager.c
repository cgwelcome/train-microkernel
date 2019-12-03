#include <hardware/timer.h>
#include <train/controller.h>
#include <train/manager.h>
#include <train/model.h>
#include <train/train.h>
#include <train/driver.h>
#include <user/io.h>
#include <utils/assert.h>
#include <utils/queue.h>

#define TRAILLING_DISTANCE      400
#define PREPARE_AHEAD_DISTANCE  700
#define REST_POSITION_ERROR     100
#define TRAIN_AROUND_REVERSE    500

extern Track singleton_track;
extern Train singleton_trains[TRAIN_COUNT];
extern int iotid;

void train_manager_setup_reverse(Train *train) {
    TrackEdge *edge = path_reverse_edge(&train->path);
    if (edge == NULL) {
        position_clear(&train->reverse_anchor);
        position_clear(&train->reverse_position);
        path_clear(&train->reverse_path);
        return;
    };
    train->reverse_anchor.node = edge->src;
    train->reverse_anchor.offset = 0;
    train->reverse_position = position_move(train->reverse_anchor, REVERSE_OVERSHOOT);
    train->reverse_path = track_cover_dist(train->reverse_anchor.node, REVERSE_OVERSHOOT);
    train->reverse_path = path_reverse(&train->reverse_path);
}

uint8_t train_manager_navigate_train(Train *train, TrackNode *dest, int32_t offset) {
    if (!singleton_track.inited || !train->inited) return 1;

    TrackPosition destination = position_move((TrackPosition) {dest, 0}, offset);
    if (destination.node == NULL) return 1;

    TrackNodeType types[TRAIN_COUNT];
    for (uint32_t i = 0; i < TRAIN_COUNT; i++) {
        types[i] = NODE_NONE;
        Train *other = &singleton_trains[i];
        if (other->inited && other->id != train->id) {
            types[i] = other->position.node->type;
            other->position.node->type = NODE_NONE;
        }
    }
    TrackPath path = track_search_path(&singleton_track, train->position.node, destination.node);
    for (uint32_t i = 0; i < TRAIN_COUNT; i++) {
        Train *other = &singleton_trains[i];
        if (other->inited && other->id != train->id) {
            other->position.node->type = types[i];
        }
    }

    if (path.dist == 0) return 1;
    train->mode = TRAIN_MODE_PATH;
    train->path = path;

    train_manager_setup_reverse(train);
    train->final_position = destination;
    return 0;
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

static bool train_manager_reserve_available(Train *train, TrackNode *node) {
    return (node->owner == UINT32_MAX || node->owner == train->id);
}

bool train_manager_will_collide_train(Train *train) {
    for (size_t i = 0; i < TRAIN_COUNT; i++) {
        Train *other = &singleton_trains[i];
        if (other->inited && other->id != train->id && train_manager_will_collide(train, other)) {
            train->blocked_train = other;
            return true;
        }
    }
    train->blocked_train = NULL;
    return false;
}

bool train_manager_will_collide_switch(Train *train) {
    TrackPath path = track_cover_dist(train->position.node, train->stop_distance + TRAILLING_DISTANCE);
    for (size_t i = 0; i < path.list.size; i++) {
        TrackEdge *edge = path.list.edges[i];
        TrackNode *dest = edge->dest;
        if (dest->type != NODE_BRANCH && dest->type != NODE_MERGE) continue;
        if (!train_manager_reserve_available(train, dest)) {
            train->blocked_switch = dest;
            return true;
        }
    }
    train->blocked_switch = NULL;
    return false;
}

static bool train_manager_will_arrive_position(Train *train, TrackPosition *position) {
    TrackPosition stop_range_start = train->position;
    int32_t offset = train->stop_distance > REST_POSITION_ERROR ? (int32_t)train->stop_distance : REST_POSITION_ERROR;
    TrackPosition stop_range_end   = position_move(train->position, offset);
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

static void train_manager_reserve_branch(Train *train, TrackNode *node) {
    assert(node->owner == node->reverse->owner);
    node->owner = train->id;
    node->reverse->owner = train->id;
}

static void train_manager_reserve_path(Train *train, TrackPath *path) {
    for (size_t i = path->index; i < path->list.size; i++) {
        TrackEdge *edge = path->list.edges[i];
        assert(edge != NULL);
        TrackNode *dest = edge->dest;
        if (dest->type == NODE_MERGE || dest->type == NODE_BRANCH) {
            if (!train_manager_reserve_available(train, dest)) break;
            train_manager_reserve_branch(train, dest);
        }
    }
}

static void train_manager_reserve_branches(Train *train) {
    // Reserve the branches around the train.
    TrackPosition around_start = position_move(train->position, -TRAIN_AROUND_REVERSE);
    TrackPosition around_end   = position_move(train->position,  TRAIN_AROUND_REVERSE);
    assert(around_start.node != NULL && around_end.node != NULL);
    for (size_t i = 0; i < singleton_track.node_count; i++) {
        TrackNode *node = &singleton_track.nodes[i];
        if (node->type == NODE_BRANCH || node->type == NODE_MERGE) {
            if (position_in_range((TrackPosition) { node, 0 }, around_start, around_end)) {
                if (train_manager_reserve_available(train, node)) {
                    train_manager_reserve_branch(train, node);
                }
            }
        }
    }

    // Reserve the branches on the coming path.
    TrackPath subpath;
    switch (train->mode) {
        case TRAIN_MODE_PATH:
            subpath = path_cover_dist(&train->path, train->stop_distance + PREPARE_AHEAD_DISTANCE);
            break;
        case TRAIN_MODE_FREE:
        case TRAIN_MODE_ROAM:
            subpath = track_cover_dist(train->position.node, train->stop_distance + PREPARE_AHEAD_DISTANCE);
            break;
    }
    train_manager_reserve_path(train, &subpath);
    if (train_manager_will_arrive_reverse(train)) {
        train_manager_reserve_path(train, &train->reverse_path);
    }
}

static void train_manager_release_branches(Train *train) {
    for (size_t i = 0; i < singleton_track.node_count; i++) {
        if (singleton_track.nodes[i].owner == train->id) {
            singleton_track.nodes[i].owner = UINT32_MAX;
        }
    }
}

static void train_manager_prepare_branches(Train *train, TrackEdgeList *list) {
    for (size_t i = 0; i < list->size; i++) {
        TrackEdge *edge = list->edges[i];
        assert(edge != NULL && edge->src != NULL);
        if (edge->src->owner != train->id) break;
        if (edge_direction(edge) != edge->src->direction) {
            controller_switch_one(edge->src->num, edge_direction(edge), 0);
        }
    }
}

static void train_manager_prepare_ahead(Train *train) {
    if (train->reverse_anchor.node != NULL) {
        if (train_manager_will_arrive_position(train, &train->reverse_anchor)) {
            TrackPath path = path_reverse(&train->reverse_path);
            TrackEdgeList reverse_list = path_filter_by_type(&path, NODE_BRANCH);
            train_manager_prepare_branches(train, &reverse_list);
        }
    }
    TrackPath subpath = path_cover_dist(&train->path, train->stop_distance + PREPARE_AHEAD_DISTANCE);
    TrackEdgeList list = path_filter_by_type(&subpath, NODE_BRANCH);
    train_manager_prepare_branches(train, &list);
}

static void train_manager_update_routing(Train *train) {
    path_next_node(&train->path, train->position.node);
    train_manager_prepare_ahead(train);
    if (train->state == TRAIN_STATE_WAIT_TRAFFIC) {
        uint8_t status = train_manager_navigate_train(train,
                train->final_position.node, (int32_t)train->final_position.offset);
        if (status == 0) {
            driver_handle_move(train, train->original_speed);
        }
    }
}

void train_manager_issue_directives() {
    if (!singleton_track.inited) return;
    for (size_t i = 0; i < TRAIN_COUNT; i++) {
        Train *train = &singleton_trains[i];
        if (!train->inited) continue;

        train_manager_release_branches(train);
        train_manager_reserve_branches(train);

        if (train->mode == TRAIN_MODE_PATH) train_manager_update_routing(train);
        train->driver_handle(train);
    }
}
