#include <string.h>
#include <train/track.h>
#include <utils/assert.h>

static Track track;

Track *track_instance() { return &track; }

TrackNode *track_find_node(Track *track, uint32_t id) {
    assert(track->inited);
    return &track->nodes[id];
}

TrackNode *track_find_node_by_name(Track *track, char *node_name) {
    assert(track->inited);

    for (size_t i = 0; i < track->node_count; i++) {
        if (strcmp(node_name, track->nodes[i].name) == 0) {
            return &track->nodes[i];
        }
    }
    throw("unknown node name");
}

TrackNode *track_find_sensor(Track *track, char module, uint32_t id) {
    assert(track->inited);
    assert(module >= 'A' && module <= 'E');
    assert(id >= 1 && id <= 16);

    return &(track->nodes[(module - 'A') * MAX_SENSOR_PER_MODULE + (int) (id - 1)]);
}

TrackNode *track_find_branch(Track *track, uint32_t switch_id) {
    assert(track->inited);

    if (switch_id > 0 && switch_id < 19) {
        return &(track->nodes[80 + 2 * (switch_id - 1)]);
    }
    if (switch_id > 0x98 && switch_id < 0x9D) {
        return &(track->nodes[80 + 2 * (switch_id + 18 - 0x99)]);
    }
    throw("unknown switch id");
}

void track_set_branch_direction(Track *track, uint32_t switch_id, uint8_t direction) {
    assert(track->inited);
    assert(direction == DIR_STRAIGHT || direction == DIR_CURVED);

    TrackNode *branch = track_find_branch(track, switch_id);
    if (branch != NULL) {
        branch->direction = direction;
    }
}

void track_path_clear(TrackPath *path) {
    path->size = 0;
    path->dist = 0;
}

void track_path_add_edge(TrackPath *path, TrackEdge *edge) {
    assert(edge != NULL);
    path->edges[path->size] = edge;
    path->dist += edge->dist;
    path->size++;
}

TrackNode *track_path_head(TrackPath *path) {
    if (path->size == 0) return NULL;
    return path->edges[path->size-1]->dest;
}

TrackEdge *track_find_next_edge(Track *track, TrackNode *src, uint8_t direction) {
    assert(track->inited);

    TrackEdge *edge = &src->edge[direction];
    if (edge->dest->type == NODE_NONE || edge->dest->type == NODE_EXIT) {
        return NULL;
    }
    return edge;
}

TrackEdge *track_find_next_current_edge(Track *track, TrackNode *src) {
    return track_find_next_edge(track, src, src->direction);
}

TrackPath track_find_next_current_sensor(Track *track, TrackNode *src) {
    assert(track->inited);

    TrackPath path;
    track_path_clear(&path);
    TrackEdge *edge = track_find_next_current_edge(track, src);
    while (edge != NULL && edge->dest->type != NODE_SENSOR) {
        track_path_add_edge(&path, edge);
        edge = track_find_next_current_edge(track, track_path_head(&path));
    }
    if (edge == NULL) {
        track_path_clear(&path);
    }
    else {
        track_path_add_edge(&path, edge);
    }
    return path;
}

void track_position_reverse(Track *track, TrackPosition *current) {
    TrackEdge *edge = track_find_next_current_edge(track, current->node);
    assert(current->offset <= edge->dist);
    current->node = edge->dest->reverse;
    current->offset = edge->dist - current->offset;
}

void track_position_move(Track *track, TrackPosition *current, int32_t offset) {
    if (offset == 0) return;
    if (offset < 0) {
        track_position_reverse(track, current);
        track_position_move(track, current, -offset);
        track_position_reverse(track, current);
        return;
    }
    current->offset += (uint32_t)offset;
    TrackEdge *edge = track_find_next_current_edge(track, current->node);
    while (current->offset >= edge->dist) {
        current->offset -= edge->dist;
        edge = track_find_next_current_edge(track, current->node);
        current->node = edge->dest;
    }
}
