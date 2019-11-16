#ifndef __TRAIN_TRACK_H__
#define __TRAIN_TRACK_H__

#include <stddef.h>
#include <stdint.h>
#include <server/train.h>

#define MAX_PATHNODE_NAME    10
#define MAX_EDGE_DEGREE       2
#define MAX_NODE_PER_TRACK  144

#define MODULE_TOTAL_NUM          5
#define MAX_SENSOR_PER_MODULE    16
#define MAX_SENSOR_NUM           80
#define MAX_EDGE_PATH           100

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

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
    TrackEdge *edges[MAX_EDGE_PATH];
    uint32_t size;
    uint32_t dist;  /** in millimetres */
} TrackPath;

typedef struct {
    TrackName name;
    size_t node_count;
    TrackNode nodes[MAX_NODE_PER_TRACK];
} Track;

/**
 * Load Train Track from a generated C function.
 */
void track_init(Track *track, TrackName name);

/**
 * Access the default track instance.
 */
Track *track_instance();

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
TrackNode *track_find_sensor(Track *track, char module, uint32_t id);

/**
 * Find branch by its switch id.
 */
TrackNode *track_find_branch(Track *track, uint32_t switch_id);

/**
 * Set branch to the specific direction
 */
void track_set_branch_direction(Track *track, uint32_t switch_id, uint8_t direction);

void track_path_clear(TrackPath *path);

void track_path_add_edge(TrackPath *path, TrackEdge *edge);

TrackNode *track_path_head(TrackPath *path);

TrackEdge *track_find_next_edge(Track *track, TrackNode *src, uint8_t direction);

TrackEdge *track_find_next_current_edge(Track *track, TrackNode *src);

TrackPath track_find_next_current_sensor(Track *track, TrackNode *src);

void track_position_reverse(Track *track, TrackPosition *current);

void track_position_move(Track *track, TrackPosition *current, int32_t offset);

#endif /*__TRAIN_TRACK_H__*/
