#include <train/track.h>
#include <utils/assert.h>
#include <utils/ppqueue.h>
#include <user/io.h>

#include <string.h>

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

TrackPath path_reverse(TrackPath *path) {
    TrackPath reverse_path;
    path_clear(&reverse_path);
    for (size_t i = 0; i < path->list.size; i++) {
        TrackEdge *edge = path->list.edges[path->list.size-1-i];
        path_add_edge(&reverse_path, edge->reverse);
    }
    return reverse_path;
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
        if (src->broken) {
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
    position->edge = NULL;
    position->offset = 0;
}

static TrackEdge *edge_reverse(TrackEdge *edge) {
    TrackNode *src = edge->dest->reverse;
    assert(src->type != NODE_EXIT);
    if (src->type == NODE_BRANCH) {
        if (src->edge[DIR_STRAIGHT].dest->reverse == edge->src) {
            return &src->edge[DIR_STRAIGHT];
        }
        if (src->edge[DIR_CURVED].dest->reverse == edge->src) {
            return &src->edge[DIR_CURVED];
        }
        throw("The edge from %s to %s failed to find reverse", edge->src->name, edge->dest->name);
    } else {
        return &src->edge[src->direction];
    }
}

TrackPosition position_reverse(TrackPosition current) {
    assert(current.edge != NULL);

    if (current.edge->dist < current.offset) {
        throw("corrupted position %s - %s: %u", current.edge->src->name, current.edge->dest->name, current.offset);
    }
    return (TrackPosition) { edge_reverse(current.edge), current.edge->dist - current.offset };
}

TrackPosition position_move(TrackPosition current, int32_t offset) {
    assert(current.edge != NULL);

    if (offset == 0) return current;
    if (offset < 0) {
        current = position_reverse(current);
        current = position_move(current, -offset);
        current = position_reverse(current);
    } else {
        current.offset += (uint32_t)offset;
        while (current.offset >= current.edge->dist) {
            if (current.edge->dest->type == NODE_EXIT) {
                current.offset = current.edge->dist;
                break;
            }
            current.offset -= current.edge->dist;
            current.edge    = node_select_next_edge(current.edge->dest);
        }
    }

    assert(current.edge != NULL);
    return current;
}

uint32_t position_dist(TrackPosition src, TrackPosition dest, uint32_t limit) {
    assert(src.edge != NULL);
    assert(dest.edge != NULL);

    if (src.edge == dest.edge) {
        if ((src.offset <= dest.offset) && (dest.offset <= src.offset + limit)) {
            return dest.offset - src.offset;
        } else {
            return UINT32_MAX;
        }
    }

    src.offset += limit;
    if (src.offset >= src.edge->dist) {
        if (src.edge->dest->type == NODE_EXIT) {
            return UINT32_MAX;
        }
        uint32_t dist = 0;
        TrackNode *node = src.edge->dest;
        uint32_t offset = src.offset - src.edge->dist;
        if (node->type == NODE_BRANCH) {
            TrackPosition straight = {&node->edge[DIR_STRAIGHT], 0};
            TrackPosition curved   = {&node->edge[DIR_CURVED],   0};
            uint32_t straight_dist = position_dist(straight, dest, offset);
            uint32_t curved_dist   = position_dist(curved, dest, offset);
            dist = straight_dist < curved_dist ? straight_dist : curved_dist;
            if (dist == UINT32_MAX) return UINT32_MAX;
        } else {
            TrackPosition ahead = {&node->edge[node->direction], 0};
            dist = position_dist(ahead, dest, offset);
            if (dist == UINT32_MAX) return UINT32_MAX;
        }
        return dist + src.edge->dist;
    }
    return UINT32_MAX;
}
