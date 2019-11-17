#include <train/manager.h>
#include <user/ui.h>

static int iotid;
static SensorAttributionList sensor_log;

void train_manager_init(int tid) {
    iotid = tid;
}

static void trainmanager_update_log(TrainSensor *sensor, Train *train, uint32_t error) {
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

void train_manager_locate_trains(TrainSensorList *list) {
    for (uint32_t i = 0; i < list->size; i++) {
        Train *train = NULL;
        trainmanager_update_log(&list->sensors[i], train, 0);
    }
    PrintSensors(iotid, &sensor_log);
}
