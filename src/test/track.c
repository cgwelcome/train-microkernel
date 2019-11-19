#include <stdlib.h>
#include <kernel.h>
#include <train/track.h>
#include <user/name.h>
#include <user/io.h>
#include <user/ui.h>

static TrainSensor find_sensor(const char *raw) {
    char module = parse_sensor_module(raw);
    uint32_t id = parse_sensor_id(raw);
    TrainSensor sensor = {
        .module = module,
        .id = id,
    };
    return sensor;
}

static uint8_t valid_sensor(TrainSensor *sensor) {
    return (sensor->id != 0 && sensor->module != 0) ? 1 : 0;
}

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
    TrainSensor sensor = find_sensor(argv[1]);
    if (!valid_sensor(&sensor)) {
        Printf(iotid, COM2, "Invalid sensor\n\r");
        return 1;
    }
    TrackNode *node = track_find_sensor(&track, &sensor);
    TrackPath path = search_path_to_next_sensor(node);
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
    TrainSensor sensor1 = find_sensor(argv[1]);
    TrainSensor sensor2 = find_sensor(argv[2]);
    if (!valid_sensor(&sensor1) && !valid_sensor(&sensor2)) {
        Printf(iotid, COM2, "Invalid sensor\n\r");
        return 1;
    }
    TrackNode *src = track_find_sensor(&track, &sensor1);
    TrackNode *dest = track_find_sensor(&track, &sensor2);
    TrackPath path = search_path_to_node(&track, src, dest);
    print_path(iotid, &path);
    Printf(iotid, COM2, "Checkpoints\n\r");
    for (uint32_t i = path.index; i < path.list.size; i++) {
        TrackNode *node = path_node_by_index(&path, i);
        if (node->type == NODE_SENSOR) {
            Printf(iotid, COM2, "%s\n\r", node->name);
            path_move(&path, node);
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
                    for (uint32_t i = path.index; i < path.list.size; i++) {
                        TrackNode *node = path_node_by_index(&path, i);
                        if (node->type == NODE_SENSOR) {
                            path_move(&path, node);
                        }
                    }
                }
            }
        }
    }
    return 0;
}
