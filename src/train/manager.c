#include <train/manager.h>
#include <train/train.h>
#include <user/ui.h>
#include <utils/queue.h>
#include <hardware/timer.h>

static int iotid;
static SensorAttributionList sensor_log;
static Queue initial_trains;

void train_manager_init(int tid) {
    iotid = tid;
    sensor_log.size = 0;
    queue_init(&initial_trains);
}

void train_manager_initialize_train(Train *train) {
    queue_push(&initial_trains, (int32_t)train->id);
}

static void train_manager_update_log(SensorAttribution *attribution) {
    if (sensor_log.size < MAX_SENSOR_LOG) {
        sensor_log.size += 1;
    }
    for (int t = MAX_SENSOR_LOG - 1; t > 0; t--) {
        sensor_log.attributions[t] = sensor_log.attributions[t - 1];
    }
    sensor_log.attributions[0] = *attribution;
}

static void train_manager_log_checkpoint(Train *train) {
    train->last_checkpoint.node = train->next_checkpoint.node;
    train->last_checkpoint.expected_time = train->next_checkpoint.expected_time;
    train->last_checkpoint.actual_time = timer_read(TIMER3);
}

static void train_manager_update_next_checkpoint(Train *train) {
    TrackPath path = search_path_to_next_sensor(train->last_checkpoint.node);
    train->next_checkpoint.node = path_head(&path);
    // Compute arrival time with path.dist, train, track
}

static void train_manager_locate_exists(Train *trains, SensorAttribution *attribution) {
    for (uint32_t i = 0; i < TRAIN_COUNT; i++) {
        if (!trains[i].inited) continue;
        if (trains[i].last_checkpoint.node == attribution->node) {
            attribution->train = &trains[i];
        } else if (trains[i].next_checkpoint.node == attribution->node) {
            attribution->train = &trains[i];
            train_manager_log_checkpoint(attribution->train);
            train_manager_update_next_checkpoint(attribution->train);
        }
    }
}

static void train_manager_locate_init(Train *train, SensorAttribution *attribution) {
    attribution->train = train;
    train->inited = 1;
    train->last_checkpoint.node = attribution->node;
    train_manager_update_next_checkpoint(train);
}


void train_manager_locate_trains(Train *trains, Track *track, TrainSensorList *list) {
    if (!track->inited) return;
    for (uint32_t i = 0; i < list->size; i++) {
        SensorAttribution attribution = {
            .node = track_find_sensor(track, &list->sensors[i]),
            .train = NULL,
            .error = 0,
        };
        train_manager_locate_exists(trains, &attribution);
        if (attribution.train == NULL && queue_size(&initial_trains) > 0) {
            Train *train = train_find(trains, (uint32_t)queue_pop(&initial_trains));
            train_manager_locate_init(train, &attribution);
        }
        train_manager_update_log(&attribution);
    }
    PrintSensors(iotid, &sensor_log);
}
