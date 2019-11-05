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

TrainPosition traingps_next_relative(TrainPath *path) {
	TrainTrackNode src = {
		.type = NODE_NONE,
	};
	TrainTrackNode dest = {
		.type = NODE_NONE,
	};
	TrainPosition position = {
		.offset = 0,
	};
	while (path->index < path->size-1 && dest.type != NODE_SENSOR) {
		src = path->nodes[path->index];
		dest = path->nodes[path->index+1];
		position.offset -= (int32_t)traingps_find_edge(&src, &dest).dist;
		path->index++;
	}
	position.base = dest;
	return position;
}

TrainPath traingps_get_subpath(TrainPath *path, uint32_t max_length) {
	TrainPath subpath = {
		.index = 0,
		.size = 0,
	};
	uint32_t length = 0;
	for (uint32_t i = path->index; i < path->size-1; i++) {
		TrainTrackNode src = path->nodes[i];
		TrainTrackNode dest = path->nodes[i+1];
		TrainTrackEdge edge = traingps_find_edge(&src, &dest);
		subpath.nodes[subpath.size] = src;
		subpath.size++;
		if (length + edge.dist > max_length) {
			return subpath;
		}
		length = length + edge.dist;
	}
	return subpath;
}

void traingps_update_next(Train *train, uint32_t time, TrainTrackStatus *status) {
	TrainPosition position;
	(void)status;
	(void)time;
	switch (train->mode) {
		case TRAINMODE_FREE:
			break;
		case TRAINMODE_PATH:
			position = traingps_next_relative(&train->path);
			train->next_position.base = position.base;
			train->next_position.offset = 0;
			// TODO: Estimate arrival time of train
			break;
	}
}

TrainJobQueue traingps_next_jobs(Train *train, TrainTrackStatus *status) {
	TrainJobQueue tjqueue = {
		.size = 0,
	};
	TrainPath visibility = traingps_get_subpath(&train->path, 9000);
	return tjqueue;
}
