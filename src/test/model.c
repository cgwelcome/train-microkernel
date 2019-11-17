#include <kernel.h>
#include <hardware/timer.h>
#include <user/name.h>
#include <user/io.h>
#include <user/train.h>
#include <train/controller.h>
#include <train/track.h>
#include <utils/assert.h>

extern Track singleton_track;

static uint64_t find_dist(TrackNode *start, TrackNode *end) {
    if (start == NULL || end == NULL) return (uint64_t) -1;
    if (start == end) return 0;

    int step = 0; uint64_t dist = 0;
    TrackNode *node = start;
    while (node != NULL && node != end) {
        if (node->type == NODE_NONE || node->type == NODE_EXIT) {
            return (uint64_t) -1;
        }
        step += 1;
        if (step >= 20) return (uint64_t) -1;
        dist += node->edge[node->direction].dist;
        node  = node->edge[node->direction].dest;
    }
    if (node != end) return (uint64_t) -1;
    return dist;
}

int test_build_model(int argc, char **argv) {
    int iotid = WhoIs(SERVER_NAME_IO);
    int clocktid = WhoIs(SERVER_NAME_CLOCK);
    int traintid = WhoIs(SERVER_NAME_TRAIN);

    TrainSpeed(traintid, 74, 10);

    TrainSensorList sensorlist;
    uint64_t total_veloc = 0;
    uint64_t veloc_count = 0;
    TrackNode *touching_sensor   = NULL;
    TrackNode *last_reach_sensor = NULL;
    uint64_t   last_reach_time   = 0;
    for (;;) {
        Delay(clocktid, 4);
        controller_read_sensors(&sensorlist);
        if (sensorlist.size > 0) {
            TrackNode *reach_sensor = track_find_sensor(&singleton_track, &sensorlist.sensors[0]);
            uint64_t   reach_time = timer_read(TIMER3);

            if (touching_sensor == reach_sensor) {
                continue;
            } else {
                touching_sensor = reach_sensor;
            }

            if (last_reach_sensor == NULL) {
                last_reach_sensor = reach_sensor;
                last_reach_time   = reach_time;
            }

            uint64_t dist = find_dist(last_reach_sensor, reach_sensor);
            uint64_t time = reach_time - last_reach_time;
            if (dist != (uint64_t) -1 && dist != 0 && time >= 3000) {
                uint32_t sv = (uint32_t) (dist * 100000 / time);
                total_veloc += (dist * 100000 * 10000 / time); veloc_count += 1;
                uint32_t av = (uint32_t) (total_veloc / 10000 / veloc_count);
                Printf(iotid, COM2, "%s\t%u\t%u\t%u.%02u\t%u.%02u\r\n", reach_sensor->name, (uint32_t) dist, (uint32_t) time, sv / 100, sv % 100, av / 100, av % 100);

                last_reach_sensor = reach_sensor;
                last_reach_time   = reach_time;
            }
        }
    }
}
