// static void train_manager_await_next_sensor(uint32_t train_id, TrackNode *sensor) {
//     assert(sensor != NULL);
//     TrackNode *next_sensor = track_find_next_sensor(&singleton_track, sensor);
//     if (next_sensor != NULL) {
//         queue_push(&await_sensors[next_sensor->num], (int) train_id);
//     }
// }

// static void train_manager_touch_train_sensor(uint32_t train_id, TrackNode *sensor) {
//     Train *train = train_find(trains, train_id);
//     if (train != NULL && train->position.node != NULL) {
//         PrintTimeDifference(io.tid, train_id, train->next_sensor_expected_time);
//         uint32_t now = timer_read(TIMER3);
//         uint32_t time = now - (uint32_t) train->prev_touch_time;
//         TrackNode *next = track_find_next_sensor(&singleton_track, train->prev_touch_node);
//         uint32_t dist = track_find_next_sensor_dist(&singleton_track, next);
//         Printf(io.tid, COM2, "\033[30;1H\033[K%s %u %u", sensor->name, time, dist * 1000 / time);
//     }
//     train_touch_sensor(train, &track, sensor);
//     train_manager_await_next_sensor(train_id, sensor);
// }

// static void train_manager_update_log(TrainSensor *sensor, Train *train) {
//     if (sensor_log.size < MAX_SENSOR_LOG) {
//         sensor_log.size += 1;
//     }
//     for (int t = MAX_SENSOR_LOG - 1; t > 0; t--) {
//         sensor_log.sensors[t] = sensor_log.sensors[t - 1];
//         sensor_log.sensors[t].train = train;
//     }
//     sensor_log.sensors[0] = *sensor;
//     sensor_log.sensors[0].train = train;
// }

// static void train_manager_update_check_sensors() {
//     TrainSensorList list;
//     controller_read_sensors(&list);
//     for (uint32_t i = 0; i < list.size; i++) {
//         /*TrackNode *sensor = track_find_sensor(&track, list.sensors[i].module, list.sensors[i].id);*/
//         Train *train = NULL;
//         /*if (sensor == NULL) {*/
//         /*} else if (queue_size(&initial_trains) > 0) {*/
//             /*uint32_t train_id = (uint32_t) queue_pop(&initial_trains);*/
//             /*train_manager_touch_train_sensor(train_id, sensor);*/
//         /*} else if (queue_size(&await_sensors[sensor->num]) > 0) {*/
//             /*uint32_t train_id = (uint32_t) queue_pop(&await_sensors[sensor->num]);*/
//             /*train_manager_touch_train_sensor(train_id, sensor);*/
//         /*}*/
//         train_manager_update_log(&list.sensors[i], train);
//     }
//     PrintSensors(io.tid, &sensor_log);
// }

// void train_manager_locate_trains() {
//     train_manager_update_check_sensors();
//     for (int i = 0; i < TRAIN_COUNT; i++) {
//         train_estimate_position(&trains[i], &track);
//     }
// }

// uint32_t trainsensor_hash(TrainSensor *sensor) {
//     return (uint32_t)(sensor->module-'A')*MAX_SENSOR_PER_MODULE+(sensor->id-1);
// }
