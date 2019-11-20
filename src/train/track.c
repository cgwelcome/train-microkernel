#include <string.h>
#include <train/track.h>
#include <utils/assert.h>
#include <utils/ppqueue.h>

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

uint8_t edge_direction(TrackEdge *edge) {
    TrackNode *node = edge->src;
    if (node->type == NODE_BRANCH) {
        return (edge == &node->edge[DIR_STRAIGHT]) ? DIR_STRAIGHT : DIR_CURVED;
    }
    else {
        return DIR_AHEAD;
    }
}

void edgelist_init(TrackEdgeList *list) {
    list->size = 0;
}

void edgelist_add(TrackEdgeList *edgelist, TrackEdge *edge) {
    if (edge != NULL) {
        edgelist->edges[edgelist->size] = edge;
        edgelist->size++;
    }
}

void edgelist_swap(TrackEdgeList *list, uint32_t i, uint32_t j) {
    TrackEdge *edge = list->edges[i];
    list->edges[i] = list->edges[j];
    list->edges[j] = edge;
}

TrackNode *edgelist_srcnode_by_index(TrackEdgeList *list, uint32_t i) {
    assert(i < list->size);
    return list->edges[i]->src;
}

TrackNode *edgelist_destnode_by_index(TrackEdgeList *list, uint32_t i) {
    assert(i < list->size);
    return list->edges[i]->dest;
}

TrackNode *edgelist_last(TrackEdgeList *list) {
    if (list->size == 0) return NULL;
    return edgelist_destnode_by_index(list, list->size-1);
}

TrackNodeList edgelist_to_nodelist(TrackEdgeList *edgelist) {
    TrackNodeList nodelist;
    nodelist_init(&nodelist);
    for (uint32_t i = 0; i < edgelist->size; i++) {
        nodelist_add(&nodelist, edgelist_destnode_by_index(edgelist, i));
    }
    return nodelist;
}

void nodelist_init(TrackNodeList *list) {
    list->size = 0;
}

void nodelist_add(TrackNodeList *list, TrackNode *node) {
    assert(list);
    assert(node);
    list->nodes[list->size] = node;
    list->size++;
}

void nodelist_append(TrackNodeList *dest, const TrackNodeList *src) {
    for (uint32_t i = 0; i < src->size; i++) {
        nodelist_add(dest, src->nodes[i]);
    }
}

void nodelist_add_reverse(TrackNodeList *list) {
    uint32_t size = list->size;
    for (uint32_t i = 0; i < size; i++) {
        nodelist_add(list, list->nodes[i]->reverse);
    }
}

void path_clear(TrackPath *path) {
    edgelist_init(&path->list);
    path->index = 0;
    path->dist = 0;
}

void path_add_edge(TrackPath *path, TrackEdge *edge) {
    assert(edge != NULL);
    edgelist_add(&path->list, edge);
    path->dist += edge->dist;
}

TrackEdge *path_edge_by_index(TrackPath *path, uint32_t i) {
    assert(i < path->list.size);
    return path->list.edges[i];
}

TrackPath path_to_greater_length(TrackPath *path, uint32_t dist) {
    TrackPath subpath;
    path_clear(&subpath);
    for (uint32_t i = path->index; i < path->list.size && path->dist < dist; i++) {
        TrackEdge *edge = path_edge_by_index(path, i);
        path_add_edge(&subpath, edge);
    }
    return subpath;
}

void path_move(TrackPath *path, TrackNode *dest) {
    assert(path->index < path->list.size);
    if (path_end(path) == dest) {
        path->index = path->list.size;
        return;
    }
    TrackNode *node = edgelist_srcnode_by_index(&path->list, path->index);
    while (node != dest) {
        path->index++;
        assert(path->index < path->list.size);
        node = edgelist_srcnode_by_index(&path->list, path->index);
    }
}

TrackNode *path_end(TrackPath *path) {
    return edgelist_last(&path->list);
}

uint8_t node_valid(TrackNode *node) {
    return (node->type != NODE_NONE && node->type != NODE_EXIT) ? 1 : 0;
}

TrackEdge *node_select_edge(TrackNode *src, uint8_t direction) {
    TrackEdge *edge = &src->edge[direction];
    return node_valid(edge->dest) ? edge : NULL;
}

TrackEdge *node_select_next_edge(TrackNode *src) {
    return node_select_edge(src, src->direction);
}

TrackEdgeList node_select_adjacent_edge(TrackNode *src) {
    TrackEdgeList adjacent;
    edgelist_init(&adjacent);
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

TrackNodeList node_select_adjacent_node(TrackNode *src) {
    TrackNodeList nodelist;
    nodelist_init(&nodelist);
    TrackEdgeList edgelist = node_select_adjacent_edge(src);
    for (uint32_t i = 0; i < edgelist.size; i++) {
        nodelist_add(&nodelist, edgelist_destnode_by_index(&edgelist, i));
    }
    return nodelist;
}

TrackPath search_path_to_next_length(TrackNode *src, uint32_t dist) {
    assert(src);
    TrackPath path;
    path_clear(&path);
    TrackEdge *edge = node_select_next_edge(src);
    while (edge != NULL && path.dist <= dist) {
        path_add_edge(&path, edge);
        edge = node_select_next_edge(path_end(&path));
    }
    return path;
}

TrackPath search_path_to_next_sensor(TrackNode *src) {
    assert(src);
    TrackPath path;
    path_clear(&path);
    TrackEdge *edge = node_select_next_edge(src);
    while (edge != NULL && edge->dest->type != NODE_SENSOR) {
        path_add_edge(&path, edge);
        edge = node_select_next_edge(path_end(&path));
    }
    if (edge == NULL) {
        path_clear(&path);
    }
    else {
        path_add_edge(&path, edge);
    }
    return path;
}

TrackPath search_path_to_next_node(TrackNode *src, TrackNode *dest) {
    TrackPath path;
    path_clear(&path);
    TrackEdge *edge = node_select_next_edge(src);
    while (edge != NULL) {
        path_add_edge(&path, edge);
        if (edge->dest == dest) break;
        edge = node_select_next_edge(path_end(&path));
    }
    if (edge == NULL) {
        path_clear(&path);
    }
    return path;
}

static TrackPath recover_path(TrackNode *src, TrackNode *dest, TrackEdge **prev) {
    TrackPath path;
    path_clear(&path);
    for (uint32_t id = dest->id; id != src->id; id = prev[id]->src->id) {
        path_add_edge(&path, prev[id]);
    }
    if (path.list.size == 0) return path;
    uint32_t i = 0;
    uint32_t j = path.list.size-1;
    while (i < j) {
        edgelist_swap(&path.list, i, j);
        i++;
        j--;
    }
    return path;
}

TrackPath search_path_to_node(Track *track, TrackNode *src, TrackNode *dest) {
    PPQueue ppqueue;
    ppqueue_init(&ppqueue);
    TrackEdge *prev[track->node_count];
    ppqueue_insert(&ppqueue, src->id, 0);
    for (uint32_t i = 0; i < track->node_count ; i++) {
        TrackNode *node = track_find_node(track, i);
        if (i != src->id && node_valid(node)) {
            ppqueue_insert(&ppqueue, i, UINT32_MAX);
        }
    }
    while (ppqueue_size(&ppqueue) > 0) {
        TrackNode *node = track_find_node(track, ppqueue_pop(&ppqueue));
        TrackEdgeList list = node_select_adjacent_edge(node);
        uint32_t node_dist = ppqueue_find_priority(&ppqueue, node->id);
        if (node_dist == UINT32_MAX) break;
        for (uint32_t i = 0; i < list.size; i++) {
            TrackEdge *edge = list.edges[i];
            uint32_t dest_dist = ppqueue_find_priority(&ppqueue, edge->dest->id);
            if (node_dist + edge->dist < dest_dist) {
                prev[edge->dest->id] = edge;
                ppqueue_change_priority(&ppqueue, edge->dest->id, node_dist + edge->dist);
            }
        }
        if (node->id == dest->id) {
            return recover_path(src, dest, prev);
        }
    }
    TrackPath path;
    path_clear(&path);
    return path;
}

void position_reverse(TrackPosition *current) {
    TrackEdge *edge = node_select_next_edge(current->node);
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
    TrackEdge *edge = node_select_next_edge(current->node);
    while (current->offset >= edge->dist) {
        current->offset -= edge->dist;
        edge = node_select_next_edge(current->node);
        current->node = edge->dest;
    }
}
