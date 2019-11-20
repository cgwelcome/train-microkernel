#include <train/manager.h>
#include <train/model.h>
#include <train/train.h>
#include <user/ui.h>
#include <utils/queue.h>
#include <hardware/timer.h>

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
}

static void train_manager_prepare_ahead(Train *train) {
    TrackPath subpath = path_to_greater_length(&train->path, SWITCH_RESERVATION_DISTANCE);
    for (uint32_t i = 0; i < subpath.list.size; i++) {
        TrackEdge *edge = edgelist_by_index(&subpath.list, i);
        TrackNode *src = edge->src;
        if (src->type == NODE_BRANCH && edge_direction(edge) != src->direction) {
            controller_switch_one(src->num, edge_direction(edge), 0);
        }
    }
}

static void train_manager_look_ahead(Train *train, uint32_t stop_distance) {
    for (size_t i = 0; i < TRAIN_COUNT; i++) {
        Train *other_train = &singleton_trains[i];
        if (!other_train->inited || other_train->id == train->id) continue;
        TrackPosition head = {
            .node = other_train->position.node,
            .offset = other_train->position.offset - stop_distance - TRAILLING_DISTANCE,
        };
        TrackPosition rear = {
            .node = other_train->position.node->reverse,
            .offset = other_train->position.offset + stop_distance + TRAILLING_DISTANCE,
        };
        if (train_close_to(train, head) != UINT32_MAX || train_close_to(train, rear) != UINT32_MAX) {
            /*if (!train->blocked) {*/
                /*train->original_speed = train->speed;*/
                /*train->blocked = true;*/
                controller_speed_one(train->id, 0, 0);
            /*}*/
        }
        /*else if (train->blocked) {*/
            /*train->blocked = false;*/
            /*controller_speed_one(train->id, train->original_speed, 0);*/
        /*}*/
    }
}

void train_manager_issue_directives() {
    for (size_t i = 0; i < TRAIN_COUNT; i++) {
        Train *train = &singleton_trains[i];
        if (train->inited) {
            if (train->trajectory) {
                path_move(&train->path, train->position.node);
                train_manager_prepare_ahead(train);
                TrackPosition position = {
                    .node = train->destination.node,
                    .offset = train->destination.offset - train->stop_distance,
                };
                if (train_close_to(train, position) != UINT32_MAX) {
                    controller_speed_one(train->id, 0, 0);
                    train->trajectory = false;
                }
            }
            train_manager_look_ahead(train, stop_distance);
        }
    }
}
