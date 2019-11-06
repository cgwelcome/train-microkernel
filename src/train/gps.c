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
        path->nodes[path->size] = &nodes[id];
        path->size++;
    }
    path->nodes[path->size] = &nodes[id];
    path->size++;
    uint32_t i = 0;
    uint32_t j = path->size-1;
    /** Reversing nodes */
    while (i < j) {
        TrainTrackNode *temp = path->nodes[i];
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
        TrainTrackNode *node = &nodes[node_id];
        switch (node->type) {
            case NODE_EXIT:
            case NODE_NONE:
                break;
            case NODE_BRANCH:
                traingps_add_undiscovered(&node->edge[DIR_STRAIGHT], &queue, discovered, prev);
                traingps_add_undiscovered(&node->edge[DIR_CURVED], &queue, discovered, prev);
                break;
            case NODE_SENSOR:
            case NODE_MERGE:
            case NODE_ENTER:
                traingps_add_undiscovered(&node->edge[DIR_AHEAD], &queue, discovered, prev);
                break;
        }
    }
}

TrainPath traingps_find(TrainTrackEdge *src, TrainTrackEdge *dest, TrainTrackStatus *status) {
    TrainPath path = {
		.index = 0,
		.size = 0,
		.src = *src,
		.dest = *dest,
	};
	traingps_find_nodes(src->src, dest->src, status->track.nodes, &path);
    return path;
}

uint32_t traingps_is_sensor(TrainTrackEdge *position) {
    if (position->src->type == NODE_SENSOR && position->dist == 0) {
        return 1;
    }
    return 0;
}

TrainSensor traingps_node_to_sensor(TrainTrackNode *node) {
    TrainSensor sensor = {
        .module = node->name[0],
    };
	sensor.id = node->num-(uint32_t)(sensor.module-'A')*MAX_SENSOR_PER_MODULE+1;
    return sensor;
}

TrainTrackEdge traingps_node_to_edge(TrainTrackNode *node) {
	TrainTrackEdge edge = {
		.src = node,
		.dest = node,
		.dist = 0,
	};
	return edge;
}

TrainTrackEdge traingps_next_dest(TrainPath *path) {
	TrainTrackNode *prev;
	TrainTrackEdge edge = traingps_node_to_edge(path->nodes[path->index]);
    while (path->index+1 < path->size-1) {
		prev = path->nodes[path->index];
		path->index++;
		edge.dest = path->nodes[path->index];
		edge.dist = traingps_find_edge(prev, edge.dest).dist;
		if (edge.dest->type == NODE_SENSOR) {
			break;
		}
    }
    return edge;
}

uint32_t traingps_node_to_switch(TrainTrackNode *node) {
    if (0x99 <= node->num && node->num <= 0x9C) {
		return node->num;
	}
	return ((node->id-80) >> 1)+1;
}

TrainTrackEdge traingps_next_dest_free(Train *train, TrainTrackStatus *status) {
	TrainTrackEdge *next = NULL;
	TrainTrackEdge edge = train->last_position;
	edge.dist = 0;
	do {
		if (edge.dest->type == NODE_BRANCH) {
			uint32_t switch_id = traingps_node_to_switch(edge.dest);
			switch (status->trainswitches[switch_id].status) {
				case TRAINSWITCHSTATUS_CURVED:
					next = &edge.dest->edge[DIR_CURVED];
					break;
				case TRAINSWITCHSTATUS_STRAIGHT:
					next = &edge.dest->edge[DIR_STRAIGHT];
					break;
			}
		}
		else {
			next = &edge.dest->edge[DIR_AHEAD];
		}
		edge.dist += next->dist;
		edge.dest = next->dest;
	} while (edge.dest->type != NODE_SENSOR && edge.dest->type != NODE_NONE && edge.dest->type != NODE_EXIT);
	return edge;
}

TrainJobQueue traingps_next_jobs(Train *train, TrainTrackStatus *status) {
	(void)train;
	(void)status;
    TrainJobQueue tjqueue;
	tjqueue_init(&tjqueue);
	switch (train->mode) {
		case TRAINMODE_FREE:
			break;
		case TRAINMODE_PATH:
			return tjqueue;
			break;
	}
	return tjqueue;
}

void traingps_update_next(Train *train, uint32_t time, TrainTrackStatus *status) {
	TrainTrackEdge displacement;
    (void)status;
    (void)time;
    switch (train->mode) {
        case TRAINMODE_FREE:
			displacement = traingps_next_dest_free(train, status);
            break;
        case TRAINMODE_PATH:
            displacement = traingps_next_dest(&train->path);
            break;
    }
	train->next_position = traingps_node_to_edge(displacement.dest);
}
