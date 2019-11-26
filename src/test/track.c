#include <stdlib.h>
#include <kernel.h>
#include <train/track.h>
#include <user/name.h>
#include <user/io.h>
#include <user/ui.h>

static void print_path(int iotid, TrackPath *path) {
    if (path->dist == 0) {
        Printf(iotid, COM2, "Path not found\n\r");
        return;
    }
    Printf(iotid, COM2, "%s -> ", path->list.edges[0]->src->name);
    for (uint32_t i = 0; i < path->list.size-1; i++) {
        Printf(iotid, COM2, "%s -> ", path->list.edges[i]->dest->name);
    }
    Printf(iotid, COM2, "%s\n\r", path->list.edges[path->list.size-1]->dest->name);
    Printf(iotid, COM2, "%d mm\n\r", path->dist);
}

int test_next_sensor(int argc, char **argv) {
    int iotid = WhoIs(SERVER_NAME_IO);
    if (argc != 2) {
        Printf(iotid, COM2, "Missing sensor\n\r");
        return 1;
    }
    Track track;
    track_init(&track, TRAIN_TRACK_A);
    TrackNode *node = track_find_node_by_name(&track, argv[1]);
    TrackPath path = search_path_to_next_type(node, NODE_SENSOR);
    print_path(iotid, &path);
    return 0;
}

int test_search_path(int argc, char **argv) {
    int iotid = WhoIs(SERVER_NAME_IO);
    if (argc != 3) {
        Printf(iotid, COM2, "Missing src dest\n\r");
        return 1;
    }
    Track track;
    track_init(&track, TRAIN_TRACK_A);
    TrackNode *src = track_find_node_by_name(&track, argv[1]);
    TrackNode *dest = track_find_node_by_name(&track, argv[2]);
    TrackPath path = search_path_to_node(&track, src, dest);
    print_path(iotid, &path);
    for (size_t i = path.index; i < path.list.size; i++) {
        TrackEdge *edge = edgelist_by_index(&path.list, i);
        if (edge_direction(edge) == DIR_REVERSE) {
            Printf(iotid, COM2, "%s%s\n\r", edge->src->name, edge->dest->name);
        }
    }
    Printf(iotid, COM2, "Checkpoints\n\r");
    for (size_t i = path.index; i < path.list.size; i++) {
        TrackEdge *edge = edgelist_by_index(&path.list, i);
        if (edge->dest->type == NODE_SENSOR) {
            path_rebase(&path, edge->dest);
            Printf(iotid, COM2, "%u\n\r", path.index);
        }
    }
    return 0;
}

int test_search_allpath(int argc, char **argv) {
    (void)argv;
    int iotid = WhoIs(SERVER_NAME_IO);
    if (argc != 1) {
        Printf(iotid, COM2, "Extra arguments\n\r");
        return 1;
    }
    Track track;
    track_init(&track, TRAIN_TRACK_A);
    for (char module1 = 'A'; module1 <= 'E'; module1++) {
        for (uint32_t id1 = 1; id1 <= 16; id1++) {
            TrainSensor sensor1 = {
                .module = module1,
                .id = id1,
            };
            for (char module2 = 'A'; module2 <= 'E'; module2++) {
                for (uint32_t id2 = 1; id2 <= 16; id2++) {
                    TrainSensor sensor2 = {
                        .module = module2,
                        .id = id2,
                    };
                    TrackNode *src = track_find_sensor(&track, &sensor1);
                    TrackNode *dest = track_find_sensor(&track, &sensor2);
                    TrackPath path = search_path_to_node(&track, src, dest);
                    for (size_t i = path.index; i < path.list.size; i++) {
                        TrackEdge *edge = edgelist_by_index(&path.list, i);
                        if (edge->dest->type == NODE_SENSOR) {
                            path_rebase(&path, edge->dest);
                        }
                    }
                }
            }
        }
    }
    return 0;
}
