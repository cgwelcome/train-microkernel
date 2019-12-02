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
    return NULL;
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

TrackNode *edge_select_src(TrackEdge *edge) {
    return (edge != NULL) ? edge->src : NULL;
}

uint8_t edge_direction(TrackEdge *edge) {
    TrackNode *src = edge->src;
    TrackNode *dest = edge->dest;
    if (src == dest->reverse) {
        return DIR_REVERSE;
    }
    if (src->type == NODE_BRANCH) {
        return (edge == &src->edge[DIR_STRAIGHT]) ? DIR_STRAIGHT : DIR_CURVED;
    } else {
        return DIR_AHEAD;
    }
}

void edgelist_init(TrackEdgeList *list) {
    list->size = 0;
}

void edgelist_add(TrackEdgeList *edgelist, TrackEdge *edge) {
    assert(edgelist->size < MAX_EDGE_LIST);
    assert(edge != NULL);
    edgelist->edges[edgelist->size] = edge;
    edgelist->size++;
}

void path_clear(TrackPath *path) {
    edgelist_init(&path->list);
    path->dist = 0;
    path->index = 0;
}

void path_add_edge(TrackPath *path, TrackEdge *edge) {
    assert(edge != NULL);
    edgelist_add(&path->list, edge);
    path->dist += edge->dist;
}

TrackNode *path_end(TrackPath *path) {
    return (path->list.size > 0) ? path->list.edges[path->list.size-1]->dest : NULL;
}

TrackEdge *path_next_node(TrackPath *path, TrackNode *dest) {
    if (path->list.size > 0 && path_end(path) == dest) {
        path->index = path->list.size;
        return NULL;
    }
    for (size_t i = path->index; i < path->list.size; i++) {
        TrackEdge *edge = path->list.edges[i];
        if (edge->src == dest) {
            path->index = i;
            return edge;
        }
    }
    return NULL;
}

TrackEdge *path_reverse_edge(TrackPath *path) {
    for (size_t i = path->index; i < path->list.size; i++) {
        TrackEdge *edge = path->list.edges[i];
        assert(edge != NULL);
        if (edge_direction(edge) == DIR_REVERSE) {
            return edge;
        }
    }
    return NULL;
}

TrackPath path_cover_dist(TrackPath *path, uint32_t dist) {
    TrackPath subpath;
    path_clear(&subpath);
    for (size_t i = path->index; i < path->list.size && subpath.dist < dist; i++) {
        TrackEdge *edge = path->list.edges[i];
        path_add_edge(&subpath, edge);
    }
    return subpath;
}

TrackEdgeList path_filter_by_type(TrackPath *path, TrackNodeType type) {
    TrackEdgeList list;
    edgelist_init(&list);
    for (size_t i = path->index; i < path->list.size; i++) {
        TrackEdge *edge = path->list.edges[i];
        assert(edge != NULL);
        if (edge->src->type == type) {
            edgelist_add(&list, edge);
        }
    }
    return list;
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
        if (src->broken || src->owner != UINT32_MAX) {
            edgelist_add(&adjacent, node_select_edge(src, src->direction));
        } else {
            edgelist_add(&adjacent, node_select_edge(src, DIR_STRAIGHT));
            edgelist_add(&adjacent, node_select_edge(src, DIR_CURVED));
        }
    } else {
        edgelist_add(&adjacent, node_select_edge(src, DIR_AHEAD));
    }
    if (src->type != NODE_BRANCH && src->type != NODE_MERGE) {
        edgelist_add(&adjacent, node_select_edge(src, DIR_REVERSE));
    }
    return adjacent;
}

static TrackPath recover_path(const TrackNode *src, const TrackNode *dest, TrackEdge **prev) {
    assert(src != NULL && dest != NULL);
    TrackPath path;
    path_clear(&path);
    for (uint32_t id = dest->id; id != src->id; id = prev[id]->src->id) {
        assert(prev[id] != NULL);
        path_add_edge(&path, prev[id]);
    }
    if (path.list.size == 0) return path;
    size_t i = 0;
    size_t j = path.list.size-1;
    while (i < j) {
        TrackEdge *temp = path.list.edges[i];
        path.list.edges[i] = path.list.edges[j];
        path.list.edges[j] = temp;
        i++;
        j--;
    }
    return path;
}
TrackPath track_search_path(Track *track, const TrackNode *src, const TrackNode *dest) {
    assert(src != NULL && dest != NULL);

    PPQueue ppqueue;
    ppqueue_init(&ppqueue);
    TrackEdge *prev[track->node_count];
    ppqueue_insert(&ppqueue, src->id, 0);
    for (size_t i = 0; i < track->node_count ; i++) {
        prev[i] = NULL;
        if (i != src->id) {
            ppqueue_insert(&ppqueue, i, UINT32_MAX);
        }
    }
    while (ppqueue_size(&ppqueue) > 0) {
        TrackNode *node = track_find_node(track, ppqueue_pop(&ppqueue));
        TrackEdgeList list = node_select_adjacent_edge(node);
        uint32_t node_dist = ppqueue_find_priority(&ppqueue, node->id);
        if (node_dist == UINT32_MAX) break;
        for (size_t i = 0; i < list.size; i++) {
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
    TrackPath path; path_clear(&path);
    return path;
}

TrackPath track_follow_path(TrackNode *src, TrackNode *dest) {
    assert(src != NULL);
    assert(dest != NULL);
    TrackPath path;
    path_clear(&path);
    TrackEdge *edge = node_select_next_edge(src);
    while (edge != NULL && edge->dest == dest) {
        path_add_edge(&path, edge);
        edge = node_select_next_edge(edge->dest);
    }
    if (edge == NULL) {
        path_clear(&path);
    } else {
        path_add_edge(&path, edge);
    }
    return path;
}

TrackPath track_cover_dist(TrackNode *src, uint32_t dist) {
    TrackPath path;
    path_clear(&path);
    TrackEdge *edge = node_select_next_edge(src);
    while (edge != NULL && path.dist <= dist) {
        path_add_edge(&path, edge);
        edge = node_select_next_edge(edge->dest);
    }
    if (edge == NULL) {
        path_clear(&path);
    } else {
        path_add_edge(&path, edge);
    }
    return path;
}


void position_clear(TrackPosition *position) {
    position->node = NULL;
    position->offset = 0;
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

    TrackEdge *edge = node_select_next_edge(current.node);
    while (edge != NULL && current.offset > edge->dist) {
        current.offset -= edge->dist;
        current.node    = edge->dest;
        edge = node_select_next_edge(current.node);
    }
    if (edge == NULL) {
        assert(current.node->type == NODE_EXIT && current.offset == 0);
        return (TrackPosition) { current.node->reverse, 0 };
    } else {
        return (TrackPosition) { edge->dest->reverse, edge->dist - current.offset };
    }
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

    assert(current.node != NULL);
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
