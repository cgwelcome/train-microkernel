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
#define REVERSE_PENALTY        1000
#define REVERSE_OVERSHOOT       170

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
    size_t size;
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
    size_t size;
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
    TrackNode *node;
    uint32_t offset;
} TrackPosition;

typedef struct {
    TrackEdgeList list;
    uint32_t dist;  /** in millimetres */
    size_t index;
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
TrackNode *edge_select_src(TrackEdge *edge);
void edgelist_init(TrackEdgeList *list);
void edgelist_add(TrackEdgeList *edgelist, TrackEdge *edge);

void path_clear(TrackPath *path);
void path_add_edge(TrackPath *path, TrackEdge *edge);
TrackNode *path_end(TrackPath *path);
TrackEdge *path_next_node(TrackPath *path, TrackNode *dest);
TrackEdge *path_reverse_edge(TrackPath *path);
TrackPath path_cover_dist(TrackPath *path, uint32_t dist);
TrackEdgeList path_filter_by_type(TrackPath *path, TrackNodeType type);

uint8_t node_valid(TrackNode *node);
TrackEdge *node_select_edge(TrackNode *src, uint8_t direction);
TrackEdge *node_select_next_edge(TrackNode *src);
TrackEdgeList node_select_adjacent_edge(TrackNode *src);

TrackPath track_search_path(Track *track, const TrackNode *src, const TrackNode *dest);
TrackPath track_follow_path(TrackNode *src, TrackNode *dest);

void position_clear(TrackPosition *position);
TrackPosition position_rebase(TrackNode *root, TrackPosition pos, uint32_t step_limit);
TrackPosition position_reverse(TrackPosition current);
TrackPosition position_move(TrackPosition current, int32_t offset);
bool position_in_range(TrackPosition pos, TrackPosition range_start, TrackPosition range_end);

#endif /*__TRAIN_TRACK_H__*/
