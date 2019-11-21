#ifndef __TRAIN_TRACK_H__
#define __TRAIN_TRACK_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <server/train.h>

#define MAX_PATHNODE_NAME        10
#define MAX_EDGE_DEGREE           3
#define MAX_NODE_PER_TRACK      144

#define MODULE_TOTAL_NUM          5
#define MAX_SENSOR_PER_MODULE    16
#define MAX_SENSOR_NUM           80
#define MAX_EDGE_LIST           280
#define MAX_NODE_LIST           280
#define MAX_EDGE_PATH           280
#define REVERSE_PENALTY        1400

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1
#define DIR_REVERSE 2

typedef struct {
    char module;
    uint32_t id;
} TrainSensor;

typedef struct {
    TrainSensor sensors[MAX_SENSOR_NUM];
    uint32_t size;
} TrainSensorList;

typedef enum {
    TRAIN_TRACK_A,
    TRAIN_TRACK_B,
} TrackName;

typedef enum {
    NODE_NONE,
    NODE_SENSOR,
    NODE_BRANCH,
    NODE_MERGE,
    NODE_ENTER,
    NODE_EXIT,
} TrackNodeType;

typedef struct PathEdge TrackEdge;

typedef struct PathNode TrackNode;

typedef struct PathEdge {
    TrackEdge *reverse;
    TrackNode *src;
    TrackNode *dest;
    uint32_t dist; /** in millimetres */
} TrackEdge;

typedef struct {
    TrackEdge *edges[MAX_EDGE_LIST];
    uint32_t size;
} TrackEdgeList;

typedef struct PathNode {
    uint32_t id;
    uint32_t owner; /* = UINT32_MAX if no one owns it */
    const char *name;
    TrackNodeType type;
    uint32_t num;

    uint8_t   broken;
    uint8_t   direction;
    TrackNode *reverse; /* same location, but opposite direction */
    TrackEdge edge[MAX_EDGE_DEGREE];
} TrackNode;

typedef struct {
    TrackNode *nodes[MAX_NODE_LIST];
    uint32_t size;
} TrackNodeList;

typedef struct {
    TrackNode *node;
    uint32_t offset;
} TrackPosition;

typedef struct {
    TrackEdgeList list;
    uint32_t index;
    uint32_t dist;  /** in millimetres */
} TrackPath;

typedef struct {
    bool inited;
    TrackName name;
    size_t node_count;
    TrackNode nodes[MAX_NODE_PER_TRACK];
} Track;

/**
 * Load Train Track from a generated C function.
 */
void track_init(Track *track, TrackName name);

/**
 * Find TrackNode by its id.
 */
TrackNode *track_find_node(Track *track, uint32_t id);

/**
 * Find TrackNode by its name.
 */
TrackNode *track_find_node_by_name(Track *track, char *node_name);

/**
 * Find sensor by its module and id.
 */
TrackNode *track_find_sensor(Track *track, TrainSensor *sensor);

/**
 * Find branch by its switch id.
 */
TrackNode *track_find_branch(Track *track, uint32_t switch_id);

/**
 * Set branch to the specific direction
 */
void track_set_branch_direction(Track *track, uint32_t switch_id, uint8_t direction);

uint8_t edge_direction(TrackEdge *edge);
void edgelist_init(TrackEdgeList *list);
void edgelist_add(TrackEdgeList *edgelist, TrackEdge *edge);
void edgelist_swap(TrackEdgeList *list, uint32_t i, uint32_t j);
TrackEdge *edgelist_by_index(TrackEdgeList *list, uint32_t i);
TrackNodeList edgelist_to_nodelist(TrackEdgeList *edgelist);
void nodelist_init(TrackNodeList *list);
void nodelist_add(TrackNodeList *list, TrackNode *node);
void nodelist_append(TrackNodeList *dest, const TrackNodeList *src);
void nodelist_add_reverse(TrackNodeList *list);

void path_clear(TrackPath *path);
void path_add_edge(TrackPath *path, TrackEdge *edge);
TrackPath path_to_greater_length(TrackPath *path, uint32_t dist);
TrackNode *path_end(TrackPath *path);
void path_move(TrackPath *path, TrackNode *dest);
TrackPosition path_to_position(TrackPath *path, uint32_t dist);

TrackEdge *node_select_edge(TrackNode *src, uint8_t direction);
TrackEdge *node_select_next_edge(TrackNode *src);
TrackEdgeList node_select_adjacent_edge(TrackNode *src);
TrackNodeList node_select_adjacent_node(TrackNode *src);

TrackPath search_path_to_next_length(TrackNode *src, uint32_t dist);
TrackPath search_path_to_next_node(TrackNode *src, TrackNode *dest);
TrackPath search_path_to_next_sensor(TrackNode *src);
TrackPath search_path_to_node(Track *track, const TrackNode *src, const TrackNode *dest);

TrackPosition position_standardize(TrackNode *node, int32_t offset);
bool position_equal(TrackPosition *pos1, TrackPosition *pos2);
TrackPosition position_rebase(TrackNode *root, TrackPosition pos, uint32_t step_limit);
TrackPosition position_reverse(TrackPosition current);
TrackPosition position_move(TrackPosition current, int32_t offset);
bool position_in_range(TrackPosition pos, TrackPosition range_start, TrackPosition range_end);

#endif /*__TRAIN_TRACK_H__*/
