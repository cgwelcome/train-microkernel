#include <train/manager.h>
#include <train/train.h>
#include <user/ui.h>
#include <utils/queue.h>
#include <hardware/timer.h>

extern Track singleton_track;
extern Train singleton_trains[TRAIN_COUNT];

void train_manager_navigate_train(uint32_t train_id, uint32_t speed, TrackNode *dest, int32_t offset) {
    (void)speed;
    (void)offset;
    Train *train = train_find(singleton_trains, train_id);
    if (!singleton_track.inited || !train->inited) return;

    TrackPath path = search_path_to_node(&singleton_track, train->last_checkpoint.node, dest);
    if (path.dist == 0) return;
    for (uint32_t i = 0; i < path.list.size; i++) {
        TrackEdge *edge = path.list.edges[i];
        if (edge->src->type == NODE_BRANCH && edge_direction(edge) != edge->src->direction) {
            controller_switch_one(edge->src->num, edge_direction(edge), 0);
        }
    }
}
