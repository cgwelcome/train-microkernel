#include <string.h>
#include <train/track.h>
#include <utils/assert.h>

static Track track;

Track *track_instance() { return &track; }

TrackNode *track_find_node(Track *track, uint32_t id) {
    if (!track->inited) return NULL;
    return &track->nodes[id];
}

TrackNode *track_find_node_by_name(Track *track, char *node_name) {
    if (!track->inited) return NULL;
    for (size_t i = 0; i < track->node_count; i++) {
        if (strcmp(node_name, track->nodes[i].name) == 0) {
            return &track->nodes[i];
        }
    }
    throw("unknown node name");
}

TrackNode *track_find_sensor(Track *track, char module, uint32_t id) {
    assert(module >= 'A' && module <= 'E');
    assert(id >= 1 && id <= 16);
    if (!track->inited) return NULL;
    return &(track->nodes[(module - 'A') * MAX_SENSOR_PER_MODULE + (int) (id - 1)]);
}

TrackNode *track_find_branch(Track *track, uint32_t switch_id) {
    if (!track->inited) return NULL;

    if (switch_id > 0 && switch_id < 19) {
        return &(track->nodes[80 + 2 * (switch_id - 1)]);
    }
    if (switch_id > 0x98 && switch_id < 0x9D) {
        return &(track->nodes[80 + 2 * (switch_id + 18 - 0x99)]);
    }
    throw("unknown switch id");
}

void track_set_branch_direction(Track *track, uint32_t switch_id, int8_t direction) {
    assert(direction == DIR_STRAIGHT || direction == DIR_CURVED);
    TrackNode *branch = track_find_branch(track, switch_id);
    if (branch != NULL) {
        branch->direction = direction;
    }
}

TrackNode *track_find_next_node(Track *track, TrackNode *node) {
    if (!track->inited) return NULL;

    if (node->type == NODE_NONE || node->type == NODE_EXIT) {
        return NULL;
    }
    return node->edge[node->direction].dest;
}

uint32_t track_find_next_node_dist(Track *track, TrackNode *node) {
    if (!track->inited) return (uint32_t) -1;
    if (node->type == NODE_NONE || node->type == NODE_EXIT) {
        return (uint32_t) -1;
    }
    return node->edge[node->direction].dist;
}

TrackNode *track_find_next_sensor(Track *track, TrackNode *node) {
    if (!track->inited) return NULL;

    TrackNode *current = track_find_next_node(track, node);
    while (current != NULL && current->type != NODE_SENSOR) {
        current = track_find_next_node(track, current);
    }
    return current;
}

uint32_t track_find_next_sensor_dist(Track *track, TrackNode *node) {
    if (!track->inited) return (uint32_t) -1;

    uint32_t dist = track_find_next_node_dist(track, node);
    TrackNode *current = track_find_next_node(track, node);
    while (current != NULL && current->type != NODE_SENSOR) {
        dist += track_find_next_node_dist(track, current);
        current = track_find_next_node(track, current);
    }
    return dist;
}

void track_position_reverse(Track *track, TrackPosition *current) {
    uint32_t dist = track_find_next_node_dist(track, current->node);
    assert(current->dist <= dist);
    current->node = track_find_next_node(track, current->node)->reverse;
    current->dist = dist - current->dist;
}

void track_position_move(Track *track, TrackPosition *current, int32_t offset) {
    if (offset == 0) return;
    if (offset < 0) {
        track_position_reverse(track, current);
        track_position_move(track, current, -offset);
        track_position_reverse(track, current);
        return;
    }
    current->dist += (uint32_t) offset;
    while (current->dist >= track_find_next_node_dist(track, current->node)) {
        current->dist -= track_find_next_node_dist(track, current->node);
        current->node = track_find_next_node(track, current->node);
    }
}
