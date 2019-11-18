#include <train/manager.h>
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

static void trainmanager_update_log(Train *train, TrainSensor *sensor, int32_t error) {
    if (sensor_log.size < MAX_SENSOR_LOG) {
        sensor_log.size += 1;
    }
    for (int t = MAX_SENSOR_LOG - 1; t > 0; t--) {
        sensor_log.attributions[t] = sensor_log.attributions[t - 1];
    }
    sensor_log.attributions[0].sensor = *sensor;
    sensor_log.attributions[0].train = train;
    sensor_log.attributions[0].error = error;
}


void train_manager_locate_trains(Train *trains, Track *track, TrainSensorList *list) {
    for (uint32_t i = 0; i < list->size; i++) {
        TrainSensor *sensor = &list->sensors[i];
        Train *train = NULL;
        int32_t error = 0;
        if (queue_size(&initial_trains) > 0) {
            train = train_find(trains, (uint32_t)queue_pop(&initial_trains));
            train->last_checkpoint.node = track_find_sensor(track, sensor);
            train->last_checkpoint.actual_time = (uint64_t)timer_read(TIMER3);
            TrackPath path = search_path_to_next_sensor(train->last_checkpoint.node);
            train->next_checkpoint.node = path_head(&path);
            // Compute arrival time with path.dist, train, track
        }
        trainmanager_update_log(train, sensor, error);
    }
    PrintSensors(iotid, &sensor_log);
}
