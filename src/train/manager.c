#include <hardware/timer.h>
#include <train/controller.h>
#include <train/manager.h>
#include <train/model.h>
#include <train/train.h>
#include <train/driver.h>
#include <user/io.h>
#include <utils/assert.h>
#include <utils/queue.h>

extern int iotid;

#define TRAILLING_DISTANCE      550
#define PREPARE_AHEAD_DISTANCE  700
#define REST_POSITION_ERROR     100
#define TRAIN_AROUND_REVERSE    500

extern Track singleton_track;
extern Train singleton_trains[TRAIN_COUNT];
extern int iotid;

static bool train_manager_will_arrive_position(Train *train, TrackPosition *position) {
    uint32_t offset = train->stop_distance > REST_POSITION_ERROR ? train->stop_distance : REST_POSITION_ERROR;
    return (position_dist(train->position, *position, offset) != UINT32_MAX);
}

static bool train_manager_reserve_available(Train *train, TrackNode *node) {
    return (node->owner == UINT32_MAX || node->owner == train->id);
}

void train_manager_setup_reverse(Train *train) {
    TrackEdge *edge = path_reverse_edge(&train->path);
    if (edge == NULL) {
        position_clear(&train->reverse_anchor);
        position_clear(&train->reverse_position);
        path_clear(&train->reverse_path);
        return;
    };
    train->reverse_anchor.edge = edge;
    train->reverse_anchor.offset = 0;

    uint32_t overshoot;
    if (train->state == TRAIN_STATE_WAIT_COMMAND && train_manager_will_arrive_position(train, &train->reverse_anchor)) {
        overshoot = 0;
    } else {
        overshoot = REVERSE_OVERSHOOT;
    }
    train->reverse_position = position_move(train->reverse_anchor, (int32_t)overshoot);
    train->reverse_path = track_cover_dist(train->reverse_anchor.edge->src, overshoot);
}

uint8_t train_manager_navigate_train(Train *train, TrackEdge *edge, int32_t offset) {
    if (!singleton_track.inited || !train->inited) return 1;

    TrackPosition destination = position_move((TrackPosition) {edge, 0}, offset);
    if (destination.edge == NULL) return 1;

    TrackNodeType train_types[TRAIN_COUNT];
    TrackNodeType branch_types[MAX_NODE_PER_TRACK];
    for (uint32_t i = 0; i < TRAIN_COUNT; i++) {
        train_types[i] = NODE_NONE;
        Train *other = &singleton_trains[i];
        if (other->inited && other->id != train->id && train_manager_reserve_available(train, other->position.edge->src)) {
            train_types[i] = other->position.edge->src->type;
            other->position.edge->src->type = NODE_NONE;
        }
    }
    for (uint32_t i = 0; i < singleton_track.node_count; i++) {
        branch_types[i] = NODE_NONE;
        TrackNode *node = &singleton_track.nodes[i];
        if (!train_manager_reserve_available(train, node)) {
            branch_types[i] = node->type;
            node->type = NODE_NONE;
        }
    }
    TrackPath path = track_search_path(&singleton_track, train->position.edge->src, destination.edge->src);
    for (uint32_t i = 0; i < TRAIN_COUNT; i++) {
        Train *other = &singleton_trains[i];
        if (other->inited && other->id != train->id && train_manager_reserve_available(train, other->position.edge->src)) {
            other->position.edge->src->type = train_types[i];
        }
    }
    for (uint32_t i = 0; i < singleton_track.node_count; i++) {
        TrackNode *node = &singleton_track.nodes[i];
        if (!train_manager_reserve_available(train, node)) {
            node->type = branch_types[i];
        }
    }
    for (uint32_t i = 0; i < singleton_track.node_count; i++) {
        TrackNode *node = &singleton_track.nodes[i];
        assert(node->type != NODE_NONE);
    }
    if (path.dist == 0) return 1;
    train->mode = TRAIN_MODE_PATH;
    train->path = path;

    train_manager_setup_reverse(train);
    train->final_position = destination;
    return 0;
}

static bool train_manager_will_collide(Train *train, Train *other) {
    assert(train->position.edge != NULL);
    assert(other->position.edge != NULL);

    uint32_t limit;
    // Case 1: train and other are in the same direction
    limit = train->stop_distance + TRAILLING_DISTANCE;
    if (position_dist(train->position, other->position, limit) != UINT32_MAX) {
        return true;
    }
    // Case 2: train and other are in the opposite direction
    limit = train->stop_distance + other->stop_distance + TRAILLING_DISTANCE + 100;
    if (position_dist(train->position, position_reverse(other->position), limit) != UINT32_MAX) {
        return true;
    }
    return false;
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
    TrackPath path = track_cover_dist(train->position.edge->src, train->stop_distance + TRAILLING_DISTANCE);
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

bool train_manager_will_arrive_final(Train *train) {
    return (train->final_position.edge != NULL) &&
        train_manager_will_arrive_position(train, &train->final_position);
}

bool train_manager_will_arrive_reverse(Train *train) {
    return (train->reverse_position.edge != NULL) &&
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
    for (size_t i = 0; i < singleton_track.node_count; i++) {
        TrackNode *node = &singleton_track.nodes[i];
        TrackPosition position = { node_select_next_edge(node), 0 };
        if (node->type == NODE_BRANCH || node->type == NODE_MERGE) {
            uint32_t dist_front  = position_dist(train->position, position, TRAIN_AROUND_REVERSE);
            uint32_t dist_behind = position_dist(position_reverse(train->position), position, TRAIN_AROUND_REVERSE);
            if (dist_front != UINT32_MAX || dist_behind != UINT32_MAX) {
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
            subpath = track_cover_dist(train->position.edge->src, train->stop_distance + PREPARE_AHEAD_DISTANCE);
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
    if (train->reverse_anchor.edge != NULL) {
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
    path_next_node(&train->path, train->position.edge->src);
    train_manager_prepare_ahead(train);
    if (train->state == TRAIN_STATE_WAIT_TRAFFIC) {
        uint8_t status = train_manager_navigate_train(train,
                train->final_position.edge, (int32_t)train->final_position.offset);
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
