#include <string.h>
#include <train/track.h>
#include <utils/assert.h>
#include <utils/ppqueue.h>
#include <user/io.h>

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
    TrackNode *src = edge->src;
    TrackNode *dest = edge->dest;
    if (src == dest->reverse) {
        return DIR_REVERSE;
    }
    if (src->type == NODE_BRANCH) {
        return (edge == &src->edge[DIR_STRAIGHT]) ? DIR_STRAIGHT : DIR_CURVED;
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

TrackEdge *edgelist_by_index(TrackEdgeList *list, uint32_t i) {
    assert(i < list->size);
    return list->edges[i];
}

TrackNode *edgelist_last(TrackEdgeList *list) {
    if (list->size == 0) return NULL;
    return edgelist_by_index(list, list->size-1)->dest;
}

TrackNodeList edgelist_to_nodelist(TrackEdgeList *edgelist) {
    TrackNodeList nodelist;
    nodelist_init(&nodelist);
    for (uint32_t i = 0; i < edgelist->size; i++) {
        nodelist_add(&nodelist, edgelist_by_index(edgelist, i)->dest);
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

TrackPath path_to_greater_length(TrackPath *path, uint32_t dist) {
    TrackPath subpath;
    path_clear(&subpath);
    for (uint32_t i = path->index; i < path->list.size && subpath.dist < dist; i++) {
        TrackEdge *edge = edgelist_by_index(&path->list, i);
        path_add_edge(&subpath, edge);
    }
    return subpath;
}

void path_move(TrackPath *path, TrackNode *dest) {
    if (path->index == path->list.size) return;
    if (path_end(path) == dest) {
        path->index = path->list.size;
        return;
    }
    TrackNode *node = edgelist_by_index(&path->list, path->index)->src;
    uint32_t count = 0;
    while (node != dest && path->index+count < path->list.size) {
        count++;
        node = edgelist_by_index(&path->list, path->index+count)->src;
    }
    if (node == dest) path->index += count;
}

TrackNode *path_end(TrackPath *path) {
    return edgelist_last(&path->list);
}

uint8_t node_valid(TrackNode *node) {
    return (node->type != NODE_NONE && node->type != NODE_EXIT) ? 1 : 0;
}

TrackEdge *node_select_edge(TrackNode *src, uint8_t direction) {
    TrackEdge *edge = &src->edge[direction];
    return node_valid(src) ? edge : NULL;
}

TrackEdge *node_select_next_edge(TrackNode *src) {
    return node_select_edge(src, src->direction);
}

TrackEdgeList node_select_adjacent_edge(TrackNode *src) {
    TrackEdgeList adjacent;
    edgelist_init(&adjacent);
    if (!node_valid(src)) return adjacent;
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
    if (src->type != NODE_BRANCH && src->type != NODE_MERGE) {
        edgelist_add(&adjacent, node_select_edge(src, DIR_REVERSE));
    }
    return adjacent;
}

TrackNodeList node_select_adjacent_node(TrackNode *src) {
    TrackNodeList nodelist;
    nodelist_init(&nodelist);
    TrackEdgeList edgelist = node_select_adjacent_edge(src);
    for (uint32_t i = 0; i < edgelist.size; i++) {
        nodelist_add(&nodelist, edgelist_by_index(&edgelist, i)->dest);
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

static TrackPath recover_path(const TrackNode *src, const TrackNode *dest, TrackEdge **prev) {
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

TrackPath search_path_to_node(Track *track, const TrackNode *src, const TrackNode *dest) {
    PPQueue ppqueue;
    ppqueue_init(&ppqueue);
    TrackEdge *prev[track->node_count];
    ppqueue_insert(&ppqueue, src->id, 0);
    for (uint32_t i = 0; i < track->node_count ; i++) {
        if (i != src->id) {
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

TrackPosition path_to_position(TrackPath *path, uint32_t dist) {
    assert(path);
    TrackPosition position;
    if (path->dist < dist) {
        position.node = NULL;
        position.offset = 0;
        return position;
    }
    uint32_t target = path->dist - dist;
    uint32_t total = 0;
    uint32_t i;
    for (i = 0; i < path->list.size; i++) {
        TrackEdge *edge = edgelist_by_index(&path->list, i);
        if (edge->dist + total > dist) break;
        total += edge->dist;
    }
    position.node = edgelist_by_index(&path->list, i)->src;
    position.offset = target - total;
    return position;
}

TrackPosition position_standardize(TrackNode *node, int32_t offset) {
    assert(node != NULL);

    TrackPosition standard;
    if (offset < 0) {
        standard.node = node->reverse;
        standard.offset = (uint32_t)offset;
    }
    else {
        standard.node = node;
        standard.offset = (uint32_t)offset;
    }
    TrackEdge *edge = node_select_next_edge(standard.node);
    while (edge != NULL && standard.offset >= edge->dist) {
        standard.offset -= edge->dist;
        standard.node = edge->dest;
        edge = node_select_next_edge(standard.node);
    }
    // Unreachable
    if (edge == NULL) {
        standard.node = NULL;
        standard.offset = 0;
    }
    return standard;
}

bool position_equal(TrackPosition *pos1, TrackPosition *pos2) {
    return (pos1->node == pos2->node && pos1->offset == pos2->offset) ? 1 : 0;
}


TrackPosition position_rebase(TrackNode *root, TrackPosition pos, uint32_t step_limit) {
    assert(root != NULL);
    assert(pos.node != NULL);

    uint32_t step = 0;
    uint32_t offset = pos.offset;
    while (root != pos.node) {
        if (root->type == NODE_EXIT) {
            return (TrackPosition) { .node = NULL, .offset = 0 };
        }
        if ((step++) >= step_limit) {
            return (TrackPosition) { .node = NULL, .offset = 0 };
        }
        offset += root->edge[root->direction].dist;
        root    = root->edge[root->direction].dest;
    }
    return (TrackPosition) { .node = root, .offset = offset };
}

TrackPosition position_reverse(TrackPosition current) {
    assert(current.node != NULL);

    if (current.node->type == NODE_EXIT) {
        assert(current.offset == 0);
        return (TrackPosition) { .node = current.node->reverse, .offset = 0 };
    }

    TrackEdge *edge = node_select_next_edge(current.node);
    assert(edge != NULL);
    if (current.offset > edge->dist) {
        throw("position_reverse: invalid position %s %u with edge dist %u", current.node->name, current.offset, edge->dist);
    }
    return (TrackPosition) {
        .node   = edge->dest->reverse,
        .offset = edge->dist - current.offset,
    };
}

TrackPosition position_move(TrackPosition current, int32_t offset) {
    assert(current.node != NULL);

    if (current.node->type == NODE_EXIT || offset == 0) return current;
    if (offset < 0) {
        current = position_reverse(current);
        current = position_move(current, -offset);
        current = position_reverse(current);
    } else {
        current.offset += (uint32_t)offset;
        TrackEdge *edge = node_select_next_edge(current.node);
        while (edge != NULL && current.offset >= edge->dist) {
            current.node    = edge->dest;
            current.offset -= edge->dist;
            edge = node_select_next_edge(current.node);
        }
        if (current.node->type == NODE_EXIT) {
            current.offset = 0;
        }
    }
    return current;
}

bool position_in_range(TrackPosition pos, TrackPosition range_start, TrackPosition range_end) {
    assert(pos.node != NULL);
    assert(range_start.node != NULL);
    assert(range_end.node != NULL);

    pos       = position_rebase(range_start.node, pos, 10);
    range_end = position_rebase(range_start.node, range_end, 10);
    if (pos.node != NULL && range_end.node != NULL) {
        return range_start.offset <= pos.offset && pos.offset <= range_end.offset;
    }
    return false;
}
