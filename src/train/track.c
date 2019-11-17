#include <string.h>
#include <train/track.h>
#include <train/trainset.h>
#include <utils/assert.h>

Track singleton_track;

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

TrackNode *track_find_sensor(Track *track, TrainSensor *sensor) {
    assert(track->inited);
    assert(sensor->module >= 'A' && sensor->module <= 'E');
    assert(sensor->id >= 1 && sensor->id <= 16);
    return &(track->nodes[(uint32_t)(sensor->module-'A')*MAX_SENSOR_PER_MODULE + (sensor->id-1)]);
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
    if (branch != NULL && branch->broken == 0) {
        branch->direction = direction;
    }
}

void path_clear(TrackPath *path) {
    path->size = 0;
    path->dist = 0;
}

void path_add_edge(TrackPath *path, TrackEdge *edge) {
    assert(edge != NULL);
    path->edges[path->size] = edge;
    path->dist += edge->dist;
    path->size++;
}

TrackNode *path_head(TrackPath *path) {
    if (path->size == 0) return NULL;
    return path->edges[path->size-1]->dest;
}

uint8_t node_valid(TrackNode *node) {
    return (node->type == NODE_NONE || node->type == NODE_EXIT) ? 0 : 1;
}

TrackEdge *node_select_edge(TrackNode *src, uint8_t direction) {
    TrackEdge *edge = &src->edge[direction];
    return node_valid(edge->dest) ? edge : NULL;
}

TrackEdge *node_select_next_current_edge(TrackNode *src) {
    return node_select_edge(src, src->direction);
}
static void edgelist_add(TrackEdgeList *edgelist, TrackEdge *edge) {
    if (edge != NULL) {
        edgelist->edges[edgelist->size] = edge;
        edgelist->size++;
    }
}

TrackEdgeList node_select_adjacent(TrackNode *src) {
    TrackEdgeList adjacent;
    adjacent.size = 0;
    if (src->type == NODE_BRANCH) {
        if (src->broken) {
            edgelist_add(&adjacent, node_select_edge(src, src->direction));
        }
        else {
            edgelist_add(&adjacent, node_select_edge(src, DIR_STRAIGHT));
            edgelist_add(&adjacent, node_select_edge(src, DIR_CURVED));
        }
    }
    else {
        edgelist_add(&adjacent, node_select_edge(src, DIR_AHEAD));
    }
    return adjacent;
}

TrackPath node_search_next_current_sensor(TrackNode *src) {
    TrackPath path;
    path_clear(&path);
    TrackEdge *edge = node_select_next_current_edge(src);
    while (edge != NULL && edge->dest->type != NODE_SENSOR) {
        path_add_edge(&path, edge);
        edge = node_select_next_current_edge(path_head(&path));
    }
    if (edge == NULL) {
        path_clear(&path);
    }
    else {
        path_add_edge(&path, edge);
    }
    return path;
}

/*TrackPath track_search_path(Track *track, TrackNode *src, TrackNode *dest) {*/
    /*TrackPath path;*/
    /*path_clear(&path);*/
    /*PPQueue ppqueue;*/
    /*ppqueue_init(&ppqueue);*/

    /*ppqueue_insert(&ppqueue, src->id, 0);*/
    /*for (uint32_t i = 0; i < track->node_count ; i++) {*/
        /*if (i != src->id) {*/
            /*ppqueue_insert(&ppqueue, i, 0);*/
        /*}*/
    /*}*/
    /*return path;*/
/*}*/

void position_reverse(TrackPosition *current) {
    TrackEdge *edge = node_select_next_current_edge(current->node);
    assert(current->offset <= edge->dist);
    current->node = edge->dest->reverse;
    current->offset = edge->dist - current->offset;
}

void position_move(TrackPosition *current, int32_t offset) {
    if (offset == 0) return;
    if (offset < 0) {
        position_reverse(current);
        position_move(current, -offset);
        position_reverse(current);
        return;
    }
    current->offset += (uint32_t)offset;
    TrackEdge *edge = node_select_next_current_edge(current->node);
    while (current->offset >= edge->dist) {
        current->offset -= edge->dist;
        edge = node_select_next_current_edge(current->node);
        current->node = edge->dest;
    }
}
