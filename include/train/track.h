#ifndef __TRAIN_TRACK_H__
#define __TRAIN_TRACK_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <server/train.h>

#define MAX_PATHNODE_NAME        10
#define MAX_EDGE_DEGREE           2
#define MAX_NODE_PER_TRACK      144

#define MODULE_TOTAL_NUM          5
#define MAX_SENSOR_PER_MODULE    16
#define MAX_SENSOR_NUM           80
#define MAX_EDGE_LIST           280
#define MAX_EDGE_PATH           100

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

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

typedef struct TrackEdgeList {
    TrackEdge *edges[MAX_EDGE_LIST];
    uint32_t size;
} TrackEdgeList;

typedef struct PathNode {
    uint32_t id;
    const char *name;
    TrackNodeType type;
    uint32_t num;

    uint8_t   broken;
    uint8_t    direction;
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

void path_clear(TrackPath *path);

void path_add_edge(TrackPath *path, TrackEdge *edge);

TrackNode *path_head(TrackPath *path);

TrackEdge *node_select_edge(TrackNode *src, uint8_t direction);

TrackEdge *node_select_next_current_edge(TrackNode *src);

TrackEdgeList node_select_adjacent(TrackNode *src);

TrackPath node_search_next_current_type(TrackNode *src, TrackNodeType type);

TrackPath track_search_path(Track *track, TrackNode *src, TrackNode *dest);

void position_reverse(TrackPosition *current);

void position_move(TrackPosition *current, int32_t offset);

#endif /*__TRAIN_TRACK_H__*/
