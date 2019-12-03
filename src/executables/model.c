#include <kernel.h>
#include <hardware/timer.h>
#include <user/clock.h>
#include <user/io.h>
#include <user/name.h>
#include <user/train.h>
#include <train/controller.h>
#include <train/track.h>
#include <utils/assert.h>

#include <string.h>

#define TRAIN_ID     1
#define TRAIN_SPEED 14

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

int exec_build_model(int argc, char **argv) {
    int iotid = WhoIs(SERVER_NAME_IO);
    int clocktid = WhoIs(SERVER_NAME_CLOCK);
    int traintid = WhoIs(SERVER_NAME_TRAIN);

    int stopflag = argc == 2 && !strcmp(argv[1], "stop");

    TrainSpeed(traintid, TRAIN_ID, TRAIN_SPEED);

    TrainSensorList sensorlist;
    uint64_t total_dist = 0;
    uint64_t total_time = 0;
    TrackNode *last_reach_sensor = NULL;
    uint64_t   last_reach_time   = 0;
    for (;;) {
        Delay(clocktid, 4);
        controller_read_sensors(&sensorlist);
        if (sensorlist.size == 1) {
            TrackNode *reach_sensor = track_find_sensor(&singleton_track, &sensorlist.sensors[0]);
            uint64_t   reach_time = timer_read(TIMER3);

            if (last_reach_sensor == NULL) {
                last_reach_sensor = reach_sensor;
                last_reach_time   = reach_time;
            }
            if (last_reach_sensor == reach_sensor) continue;

            if (stopflag) {
                if (reach_sensor->id == 36) {
                    TrainSpeed(traintid, TRAIN_ID, 0);
                }
            }

            uint64_t dist = find_dist(last_reach_sensor, reach_sensor);
            uint64_t time = reach_time - last_reach_time;
            if (dist != (uint64_t) -1 && dist != 0 && time >= 1000) {
                total_dist += dist;
                total_time += time;
                uint32_t sv = (uint32_t) (dist * 100000 / time);
                uint32_t tv = (uint32_t) (total_dist * 100000 / total_time);
                Printf(iotid, COM2, "%s\t%s\t%u\t%u\t%u.%02u\t%u.%02u\r\n",
                        last_reach_sensor->name, reach_sensor->name,
                        (uint32_t) dist, (uint32_t) time, sv / 100, sv % 100, tv / 100, tv % 100);

                last_reach_sensor = reach_sensor;
                last_reach_time   = reach_time;
            }
        }
    }
}
