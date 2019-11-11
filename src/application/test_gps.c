// #include <train/gps.h>
// #include <train/track.h>
// #include <utils/bwio.h>
// #include <utils/assert.h>
// #include <user/tasks.h>

// void gps_test_find() {
//     TrackStatus status;
//     track_init(TRAIN_TRACK_B);
//     TrackEdge src = *(track_search("A1")->edge);
// 	src.dist = 0;
//     TrackEdge dest = *(track_search("E1")->edge);
// 	dest.dist = 0;

//     TrainPath path = gps_find(&src, &dest, &status);
//     for (uint32_t i = 0; i < path.size; i++) {
//         bwprintf(COM2, "%s -> ", path.nodes[i]->name);
//     }
//     Exit();
// }

// void gps_test_subpath() {
//     TrainSensor src_sensor = {
//         .id = 1,
//         .module = 'A',
//     };
//     TrainSensor dest_sensor = {
//         .id = 1,
//         .module = 'E',
//     };
//     TrackStatus status;
//     track_init(TRAIN_TRACK_B);
//     assert(trainsensor_hash(&src_sensor) == 0);
//     assert(trainsensor_hash(&dest_sensor) == 64);
//     TrackEdge src = *(track_search("A1")->edge);
// 	src.dist = 0;
//     TrackEdge dest = *(track_search("E1")->edge);
// 	dest.dist = 0;
//     TrainPath path = gps_find(&src, &dest, &status);
//     for (uint32_t i = 0; i < path.size; i++) {
//         bwprintf(COM2, "%s -> ", path.nodes[i]->name);
//     }
//     bwprintf(COM2, "\n\r");
//     for (uint32_t i = 0; i < 12; i++) {
// 		TrackEdge edge = gps_next_dest(&path);
//         bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
//         bwprintf(COM2, "\n\r");
//     }
//     Exit();
// }

// void gps_test_middle_free() {
// 	Train train;
//     TrackStatus status;
// 	track_init(TRAIN_TRACK_B);
//     TrackEdge dest = gps_node_to_edge(track_search("E1"));
// 	train.last_position = dest;
//     for (uint32_t id = 1; id <= 18; id++) {
// 		status.trainswitches[id].id = id;
// 		status.trainswitches[id].status = DIR_CURVED;
//     }
//     for (uint32_t id = 0x99; id <= 0x9C; id++) {
// 		status.trainswitches[id].id = id;
// 		status.trainswitches[id].status = DIR_CURVED;
//     }
// 	TrackEdge edge = gps_next_dest_free(&train, &status);
// 	bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
// 	bwprintf(COM2, "\n\r");
// 	status.trainswitches[154].id = 154;
// 	status.trainswitches[154].status = DIR_STRAIGHT;
// 	edge = gps_next_dest_free(&train, &status);
// 	bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
// 	bwprintf(COM2, "\n\r");
// 	status.trainswitches[153].id = 153;
// 	status.trainswitches[153].status = DIR_STRAIGHT;
// 	edge = gps_next_dest_free(&train, &status);
// 	bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
// 	bwprintf(COM2, "\n\r");
// 	status.trainswitches[154].id = 154;
// 	status.trainswitches[154].status = DIR_CURVED;
// 	edge = gps_next_dest_free(&train, &status);
// 	bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
// 	bwprintf(COM2, "\n\r");
// 	Exit();
// }

// void gps_test_next_dest_free() {
// 	Train train;
//     TrackStatus status;
// 	track_init(TRAIN_TRACK_B);
//     TrackNode *node = track_search("A13");
//     TrackEdge dest = gps_node_to_edge(node);
// 	train.last_position = dest;
//     for (uint32_t id = 1; id <= 18; id++) {
// 		status.trainswitches[id].id = id;
// 		status.trainswitches[id].status = DIR_CURVED;
//     }
// 	TrackEdge edge = gps_next_dest_free(&train, &status);
// 	bwprintf(COM2, "%s-%s:%d", edge.src->name, edge.dest->name, edge.dist);
// 	bwprintf(COM2, "\n\r");
// 	Exit();
// }

// void gps_test_root_task() {
// 	/*Create(1, &gps_test_find);*/
//     /*Create(2, &gps_test_subpath);*/
//     Create(2, &gps_test_next_dest_free);
// 	Exit();
// }
