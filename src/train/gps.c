#include <train/gps.h>
#include <utils/queue.h>

// static TrackEdge gps_find_edge(TrackNode *src, TrackNode *dest) {
//     if (src->type != NODE_BRANCH) {
//         return src->edge[DIR_AHEAD];
//     }
//     if (src->edge[DIR_STRAIGHT].src->id == dest->id) {
//         return src->edge[DIR_STRAIGHT];
//     }
//     else {
//         return src->edge[DIR_CURVED];
//     }
// }

// static void gps_recover_path(TrainPath *path, TrackNode *dest, uint32_t *prev, TrackNode *nodes) {
//     uint32_t id;
//     for (id = dest->id; prev[id] != MAX_NODE_PER_TRACK; id = prev[id]) {
//         path->nodes[path->size] = &nodes[id];
//         path->size++;
//     }
//     path->nodes[path->size] = &nodes[id];
//     path->size++;
//     uint32_t i = 0;
//     uint32_t j = path->size-1;
//     /** Reversing nodes */
//     while (i < j) {
//         TrackNode *temp = path->nodes[i];
//         path->nodes[i] = path->nodes[j];
//         path->nodes[j] = temp;
//         i++;
//         j--;
//     }
// }

// static void gps_add_undiscovered(TrackEdge *edge, Queue *queue, uint32_t *discovered, uint32_t *prev) {
//     if (discovered[edge->dest->id] == 0) {
//         discovered[edge->dest->id] = 1;
//         queue_push(queue, (int)edge->dest->id);
//         prev[edge->dest->id] = edge->src->id;
//     }
// }

// // Use BFS to find path, TODO: Implement using Djikstra
// static void gps_find_nodes(TrackNode *src, TrackNode *dest, TrackNode *nodes, TrainPath *path) {
//     Queue queue;
//     queue_init(&queue);
//     uint32_t prev[MAX_NODE_PER_TRACK];
//     uint32_t discovered[MAX_NODE_PER_TRACK];
//     for (uint32_t i = 0; i < MAX_NODE_PER_TRACK; i++) {
//         discovered[i] = 0;
//         prev[i] = MAX_NODE_PER_TRACK;
//     }
//     queue_push(&queue, (int32_t)src->id);
//     discovered[src->id] = 1;
//     while (queue_size(&queue) > 0) {
//         uint32_t node_id = (uint32_t)queue_pop(&queue);
//         if (node_id == dest->id) {
//             gps_recover_path(path, dest, prev, nodes);
//             return;
//         }
//         TrackNode *node = &nodes[node_id];
//         switch (node->type) {
//             case NODE_EXIT:
//             case NODE_NONE:
//                 break;
//             case NODE_BRANCH:
//                 gps_add_undiscovered(&node->edge[DIR_STRAIGHT], &queue, discovered, prev);
//                 gps_add_undiscovered(&node->edge[DIR_CURVED], &queue, discovered, prev);
//                 break;
//             case NODE_SENSOR:
//             case NODE_MERGE:
//             case NODE_ENTER:
//                 gps_add_undiscovered(&node->edge[DIR_AHEAD], &queue, discovered, prev);
//                 break;
//         }
//     }
// }

// TrainPath gps_find(TrackEdge *src, TrackEdge *dest, TrackStatus *status) {
//     TrainPath path = {
// 		.index = 0,
// 		.size = 0,
// 		.src = *src,
// 		.dest = *dest,
// 	};
// 	gps_find_nodes(src->src, dest->src, status->track.nodes, &path);
//     return path;
// }

// uint32_t gps_is_sensor(TrackEdge *position) {
//     if (position->src->type == NODE_SENSOR && position->dist == 0) {
//         return 1;
//     }
//     return 0;
// }

// TrainSensor gps_node_to_sensor(TrackNode *node) {
//     TrainSensor sensor = {
//         .module = node->name[0],
//     };
// 	sensor.id = node->num-(uint32_t)(sensor.module-'A')*MAX_SENSOR_PER_MODULE+1;
//     return sensor;
// }

// TrackEdge gps_node_to_edge(TrackNode *node) {
// 	TrackEdge edge = {
// 		.src = node,
// 		.dest = node,
// 		.dist = 0,
// 	};
// 	return edge;
// }

// TrackEdge gps_next_dest(TrainPath *path) {
// 	TrackNode *prev;
// 	TrackEdge edge = gps_node_to_edge(path->nodes[path->index]);
//     while (path->index+1 < path->size-1) {
// 		prev = path->nodes[path->index];
// 		path->index++;
// 		edge.dest = path->nodes[path->index];
// 		edge.dist = gps_find_edge(prev, edge.dest).dist;
// 		if (edge.dest->type == NODE_SENSOR) {
// 			break;
// 		}
//     }
//     return edge;
// }

// TrackEdge gps_next_dest_free(Train *train, TrackStatus *status) {
// 	TrackEdge *next = NULL;
// 	TrackEdge edge = train->last_position;
// 	edge.dist = 0;
// 	do {
// 		if (edge.dest->type == NODE_BRANCH) {
// 			uint32_t switch_id = edge.dest->num;
// 			switch (status->trainswitches[switch_id].status) {
// 				case DIR_CURVED:
// 					next = &edge.dest->edge[DIR_CURVED];
// 					break;
// 				case DIR_STRAIGHT:
// 					next = &edge.dest->edge[DIR_STRAIGHT];
// 					break;
// 			}
// 		} else {
// 			next = &edge.dest->edge[DIR_AHEAD];
// 		}
// 		edge.dist += next->dist;
// 		edge.dest = next->dest;
// 	} while (edge.dest->type != NODE_SENSOR && edge.dest->type != NODE_NONE && edge.dest->type != NODE_EXIT);
// 	return edge;
// }

// void gps_schedule_path(Train *train, TrackStatus *status) {
// 	(void)train;
// 	(void)status;
//     if (train->mode == TRAINMODE_FREE) return;
//     // TODO: send a list of jobs to the scheduler.
//     return;
// }

// void gps_update_next(Train *train, uint32_t time, TrackStatus *status) {
// 	TrackEdge displacement;
//     (void)status;
//     (void)time;
//     switch (train->mode) {
//         case TRAINMODE_FREE:
// 			displacement = gps_next_dest_free(train, status);
//             break;
//         case TRAINMODE_PATH:
//             displacement = gps_next_dest(&train->path);
//             break;
//     }
// 	train->next_position = gps_node_to_edge(displacement.dest);
// }
