#ifndef __TRAIN_GPS_H__
#define __TRAIN_GPS_H__

#include <train/manager.h>

/**
 * Find the path from source to destination based on traffic
 */
TrainPath gps_find(TrainTrackEdge *src, TrainTrackEdge *dest, TrainTrackStatus *status);

/**
 * Check wheter a position is on top of a sensor
 */
uint32_t gps_is_sensor(TrainTrackEdge *position);

/**
 * Make self-referencing edge to node with offset zero
 */
TrainTrackEdge gps_node_to_edge(TrainTrackNode *node);

/**
 * Convert a node to sensor
 */
TrainSensor gps_node_to_sensor(TrainTrackNode *node);

/**
 * Convert from a node to a switch id
 */
uint32_t gps_node_to_switch(TrainTrackNode *node);

/**
 * Get the next sensor, with dist, and dest = sensor in Path Mode
 */
TrainTrackEdge gps_next_dest(TrainPath *path);

/**
 * Get the next sensor, with dist, and dest = sensor in Free Runnning Mode
 */
TrainTrackEdge gps_next_dest_free(Train *train, TrainTrackStatus *status);

/**
 * Return any train job that is need to be schedule, i.e. stop a train after
 * a time delay
 */
void gps_schedule_path(Train *train, TrainTrackStatus *status);

/**
 * Update the train next position, and the Estimated time of clock
 * to the next dest. The time depends on the train position, velocity,
 * acceleration, any other interferences from other trains, and track calibration
 */
void gps_update_next(Train *train, uint32_t time, TrainTrackStatus *status);

#endif /*__TRAIN_GPS_H__*/
