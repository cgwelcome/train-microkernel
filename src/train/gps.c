#include <stdint.h>
#include <train/gps.h>
#include <utils/queue.h>

static TrainTrackEdge traingps_find_edge(TrainTrackNode *src, TrainTrackNode *dest) {
    if (src->type != NODE_BRANCH) {
        return src->edge[DIR_AHEAD];
    }
    if (src->edge[DIR_STRAIGHT].src->id == dest->id) {
        return src->edge[DIR_STRAIGHT];
    }
    else {
        return src->edge[DIR_CURVED];
    }
}

static void traingps_recover_path(TrainPath *path, TrainTrackNode *dest, uint32_t *prev, TrainTrackNode *nodes) {
    uint32_t id;
    for (id = dest->id; prev[id] != MAX_NODE_PER_TRACK; id = prev[id]) {
        path->nodes[path->size] = nodes[id];
        path->size++;
    }
    path->nodes[path->size] = nodes[id];
    path->size++;
    uint32_t i = 0;
    uint32_t j = path->size-1;
    /** Reversing nodes */
    while (i < j) {
        TrainTrackNode temp = path->nodes[i];
        path->nodes[i] = path->nodes[j];
        path->nodes[j] = temp;
        i++;
        j--;
    }
}

static void traingps_add_undiscovered(TrainTrackEdge *edge, Queue *queue, uint32_t *discovered, uint32_t *prev) {
    if (discovered[edge->dest->id] == 0) {
        discovered[edge->dest->id] = 1;
        queue_push(queue, (int)edge->dest->id);
        prev[edge->dest->id] = edge->src->id;
    }
}

// Use BFS to find path, TODO: Implement using Djikstra
static void traingps_find_nodes(TrainTrackNode *src, TrainTrackNode *dest, TrainTrackNode *nodes, TrainPath *path) {
    Queue queue;
    queue_init(&queue);
    uint32_t prev[MAX_NODE_PER_TRACK];
    uint32_t discovered[MAX_NODE_PER_TRACK];
    for (uint32_t i = 0; i < MAX_NODE_PER_TRACK; i++) {
        discovered[i] = 0;
        prev[i] = MAX_NODE_PER_TRACK;
    }
    queue_push(&queue, (int32_t)src->id);
    discovered[src->id] = 1;
    while (queue_size(&queue) > 0) {
        uint32_t node_id = (uint32_t)queue_pop(&queue);
        if (node_id == dest->id) {
            traingps_recover_path(path, dest, prev, nodes);
            return;
        }
        TrainTrackNode node =  nodes[node_id];
        switch (node.type) {
            case NODE_EXIT:
            case NODE_NONE:
                break;
            case NODE_BRANCH:
                traingps_add_undiscovered(&node.edge[DIR_STRAIGHT], &queue, discovered, prev);
                traingps_add_undiscovered(&node.edge[DIR_CURVED], &queue, discovered, prev);
                break;
            case NODE_SENSOR:
            case NODE_MERGE:
            case NODE_ENTER:
                traingps_add_undiscovered(&node.edge[DIR_AHEAD], &queue, discovered, prev);
                break;
        }
    }
}

TrainPath traingps_find(TrainPosition *src, TrainPosition *dest, TrainTrackStatus *status) {
    TrainPath path = {
        .index = 0,
        .size = 0,
        .dest = *dest,
    };
    traingps_find_nodes(&src->base, &dest->base, status->track.nodes, &path);
    return path;
}

uint32_t traingps_is_sensor(TrainPosition *position) {
    if (position->base.type == NODE_SENSOR && position->offset == 0) {
        return 1;
    }
    return 0;
}

TrainSensor traingps_node_to_sensor(TrainTrackNode *node) {
    TrainSensor sensor = {
        .id = (uint32_t)(node->name[1]-'0'),
        .module = node->name[0],
    };
    return sensor;
}

/**
 * Pop until the next position of a sensor, or the last position
 */
static TrainPosition traingps_pop_next_position(TrainPath *path) {
    TrainTrackNode src, dest;
    TrainPosition position = {
        .offset = 0,
    };
    while (path->index < path->size) {
        src = path->nodes[path->index];
        dest = path->nodes[path->index+1];
        position.offset -= (int32_t)traingps_find_edge(&src, &dest).dist;
        path->index++;
        if (dest.type == NODE_SENSOR) {
            break;
        }
    }
    position.base = dest;
    return position;
}

static TrainPath traingps_get_subpath(TrainPath *path, uint32_t max_length) {
    TrainPath subpath = {
        .index = 0,
        .size = 0,
    };
    uint32_t length = 0;
    for (uint32_t i = path->index; i < path->size-1; i++) {
        // Read the next edge of the path
        TrainTrackNode src = path->nodes[path->index];
        TrainTrackNode dest = path->nodes[path->index+1];
        TrainTrackEdge edge = traingps_find_edge(&src, &dest);
        if (length + edge.dist > max_length) {
            return subpath;
        }
        subpath.nodes[subpath.size] = dest;
        subpath.size++;
        length = length + edge.dist;
    }
    return subpath;
}

void traingps_update_next(Train *train, uint32_t time, TrainTrackStatus *status) {
    (void)status;
    (void)time;
    switch (train->mode) {
        case TRAINMODE_FREE:
            break;
        case TRAINMODE_PATH:
            train->next_position = traingps_pop_next_position(&train->path);
            break;
    }
}

TrainJobQueue traingps_next_jobs(Train *train, TrainTrackStatus *status) {
    (void)status;
    (void)train;
    TrainJobQueue tjqueue = {
        .size = 0,
    };
    return tjqueue;
}
