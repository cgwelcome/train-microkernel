#ifndef __TRAIN_TRACK_H__
#define __TRAIN_TRACK_H__

#include <stdint.h>
#include <server/train.h>

#define MAX_PATHNODE_NAME    10
#define MAX_EDGE_DEGREE       2
#define MAX_NODE_PER_TRACK  140
#define MAX_EDGE_PER_TRACK  280

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

typedef enum {
  NODE_NONE,
  NODE_SENSOR,
  NODE_BRANCH,
  NODE_MERGE,
  NODE_ENTER,
  NODE_EXIT,
} TrainTrackNodeType;

typedef struct PathEdge {
  struct PathEdge *reverse;
  struct PathNode *src;
  struct PathNode *dest;
  /* in millimetres */
  uint32_t dist;
} TrainTrackEdge;

typedef struct PathNode {
  uint32_t id;
  const char *name;
  TrainTrackNodeType type;
  uint32_t num;
  /* same location, but opposite direction */
  struct PathNode *reverse;
  struct PathEdge edge[MAX_EDGE_DEGREE];
} TrainTrackNode;

typedef struct {
    TrainTrackType type;
    TrainTrackNode nodes[MAX_NODE_PER_TRACK];
} TrainTrack;

/**
 * Load Train Track A from a generated C function
 */
void init_tracka(TrainTrackNode *track);

/**
 * Load Train Track B from a generated C function
 */
void init_trackb(TrainTrackNode *track);

#endif /*__TRAIN_TRACK_H__*/
