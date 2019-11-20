#include <train/train.h>
#include <hardware/timer.h>
#include <utils/assert.h>

Train singleton_trains[TRAIN_COUNT];

const uint32_t train_ids[TRAIN_COUNT] = { 1, 24, 58, 74, 78, 79 };

void train_init(Train *train, uint32_t id) {
    train->inited = false;
    train->id = id;
    train->speed = 0;
    train->velocity = 0;
    train->position.node = NULL;
    train->position.offset= 0;
    train->last_position_update_time = 0;
    train->last_checkpoint.node = NULL;
    train->next_checkpoint.node = NULL;
}

uint32_t train_id_to_index(uint32_t train_id) {
    if (train_id ==  1) return 0;
    if (train_id == 24) return 1;
    if (train_id == 58) return 2;
    if (train_id == 74) return 3;
    if (train_id == 78) return 4;
    if (train_id == 79) return 5;
    throw("unknown train id");
}

uint32_t train_index_to_id(uint32_t index) {
    return train_ids[index];
}

Train *train_find(Train *trains, uint32_t train_id) {
    return &trains[train_id_to_index(train_id)];
}

static TrackPosition rebase_position(TrackNode *root, TrackPosition pos) {
    uint32_t offset = pos.offset;
    uint32_t step = 0;
    while (root != pos.node) {
        if (root->type == NODE_EXIT) {
            return (TrackPosition) { .node = NULL, .offset = 0 };
        }
        if ((step++) >= 5) {
            return (TrackPosition) { .node = NULL, .offset = 0 };
        }
        offset += root->edge[root->direction].dist;
        root    = root->edge[root->direction].dest;
    }
    return (TrackPosition) { .node = root, .offset = offset };
}

static uint32_t train_close_dist (TrackPosition pos, TrackPosition dest) {
    if (pos.offset - 50 <= dest.offset && dest.offset <= pos.offset + 50) {
        return dest.offset > pos.offset ? (dest.offset - pos.offset) : (pos.offset - dest.offset);
    }
    return UINT32_MAX;
}

uint32_t train_close_to(Train *train, TrackPosition dest) {
    TrackPosition current = train->position;
    if (current.node == NULL || dest.node == NULL) {
        return UINT32_MAX;
    }
    TrackPosition rebased_dest = rebase_position(current.node, dest);
    if (rebased_dest.node != NULL) {
        return train_close_dist(current, rebased_dest);
    }
    TrackPosition rebased_current = rebase_position(dest.node, current);
    if (rebased_current.node != NULL) {
        return train_close_dist(rebased_current, dest);
    }
    return UINT32_MAX;
}
