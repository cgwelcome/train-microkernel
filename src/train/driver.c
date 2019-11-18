#include <train/train.h>
#include <train/track.h>
#include <train/controller.h>

extern Track singleton_track;
extern Train singleton_trains[TRAIN_COUNT];

void driver_navigate(uint32_t train_id, uint32_t speed, TrainSensor *sensor, int32_t offset) {
    (void)speed;
    (void)offset;
    Train *train = train_find(singleton_trains, train_id);
    if (!singleton_track.inited || !train->inited) return;
    TrackNode *dest = track_find_sensor(&singleton_track, sensor);
    TrackPath path = search_path_to_node(&singleton_track, train->last_checkpoint.node, dest);
    if (path.dist == 0) return;
    for (uint32_t i = 0; i < path.list.size; i++) {
        TrackEdge *edge = path.list.edges[i];
        if (edge->src->type == NODE_BRANCH && edge_direction(edge) != edge->src->direction) {
            controller_switch_one(edge->src->num, edge_direction(edge), 0);
        }
    }
}
