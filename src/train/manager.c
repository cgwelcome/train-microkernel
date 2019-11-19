#include <train/manager.h>
#include <train/train.h>
#include <user/ui.h>
#include <utils/queue.h>
#include <hardware/timer.h>

extern Track singleton_track;
extern Train singleton_trains[TRAIN_COUNT];

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
    train->next_checkpoint.node = path_end(&path);
    // Compute arrival time with path.dist, train, track
}

static void train_manager_locate_exists(SensorAttribution *attribution) {
    for (uint32_t i = 0; i < TRAIN_COUNT; i++) {
        if (!singleton_trains[i].inited) continue;
        if (singleton_trains[i].last_checkpoint.node == attribution->node) {
            attribution->train = &singleton_trains[i];
        } else if (singleton_trains[i].next_checkpoint.node == attribution->node) {
            attribution->train = &singleton_trains[i];
            train_manager_log_checkpoint(attribution->train);
            train_manager_update_next_checkpoint(attribution->train);
        }
    }
}

static void train_manager_locate_init(Train *train, SensorAttribution *attribution) {
    attribution->train = train;
    train->inited = true;
    train->last_checkpoint.node = attribution->node;
    train_manager_update_next_checkpoint(train);
}

void train_manager_locate_trains(TrainSensorList *list) {
    if (!singleton_track.inited) return;
    for (uint32_t i = 0; i < list->size; i++) {
        SensorAttribution attribution = {
            .node = track_find_sensor(&singleton_track, &list->sensors[i]),
            .train = NULL,
            .error = 0,
        };
        train_manager_locate_exists(&attribution);
        if (attribution.train == NULL && queue_size(&initial_trains) > 0) {
            Train *train = train_find(singleton_trains, (uint32_t)queue_pop(&initial_trains));
            train_manager_locate_init(train, &attribution);
        }
        train_manager_update_log(&attribution);
    }
    PrintSensors(iotid, &sensor_log);
}

void train_manager_navigate_train(uint32_t train_id, uint32_t speed, TrainSensor *sensor, int32_t offset) {
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
