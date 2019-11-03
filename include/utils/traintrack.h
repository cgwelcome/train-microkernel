#ifndef __UTILS_TRAINTRACK_H__
#define __UTILS_TRAINTRACK_H__

#include <stdint.h>

#define MAX_PATHNODE_NAME 10
#define MAX_EDGE_DEGREE 2

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
  int dist;
} TrackEdge;

typedef struct PathNode {
  const char *name;
  TrainTrackNodeType type;
  /* sensor or switch number */
  uint32_t num;
  /* same location, but opposite direction */
  struct PathNode *reverse;
  struct PathEdge edge[MAX_EDGE_DEGREE];
} TrainTrackNode;

void init_tracka(TrainTrackNode *track);

void init_trackb(TrainTrackNode *track);

#endif /*__UTILS_TRAINTRACK_H__*/
