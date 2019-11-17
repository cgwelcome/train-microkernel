#include <stdlib.h>
#include <kernel.h>
#include <train/track.h>
#include <user/name.h>
#include <user/io.h>
#include <user/ui.h>

int test_next_sensor(int argc, char **argv) {
    int iotid = WhoIs(SERVER_NAME_IO);
    if (argc != 2) {
        Printf(iotid, COM2, "Missing sensor\n\r");
        return 1;
    }
    Track track;
    track_init(&track, TRAIN_TRACK_A);
    char module = parse_sensor_module(argv[1]);
    uint32_t id = parse_sensor_id(argv[1]);
    if (module == 0 || id == 0) {
        Printf(iotid, COM2, "Invalid sensor\n\r");
        return 1;
    }
    TrainSensor sensor = {
        .module = module,
        .id = id,
    };
    TrackNode *node = track_find_sensor(&track, &sensor);
    TrackPath path = node_search_next_current_sensor(node);
    if (path.dist == 0) {
        Printf(iotid, COM2, "Path not found\n\r");
        return 0;
    }
    for (uint32_t i = 0; i < path.size-1; i++) {
        Printf(iotid, COM2, "%s -> ", path.edges[i]->dest->name);
    }
    Printf(iotid, COM2, "%s\n\r", path.edges[path.size-1]->dest->name);
    Printf(iotid, COM2, "%d mm\n\r", path.dist);
    return 0;
}
