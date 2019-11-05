#ifndef __TRAIN_GPS_H__
#define __TRAIN_GPS_H__

#include <train/job.h>
#include <train/manager.h>

/**
 * Calculate the path length from src
 */
uint64_t traingps_length(TrainPath *path, TrainPosition *src);

/**
 * Find the path from src to destination based on traffic
 */
TrainPath traingps_find(TrainPosition *src, TrainPosition *dest, TrainTrackStatus *status);

/**
 * Check wheter a position is on top of a sensor
 */
uint32_t traingps_is_sensor(TrainPosition *position);

/**
 * Convert a node to sensor
 */
TrainSensor traingps_node_to_sensor(TrainTrackNode *node);

/**
 * Update the train next position, and the Estimated time of clock
 * to the next dest. The time depends on the train position, velocity,
 * acceleration, any other interferences from other trains, and track calibration
 */
void traingps_update_next(Train *train, uint32_t time, TrainTrackStatus *status);

/**
 * Return any train job that is need to be schedule, i.e. stop a train after
 * a time delay
 */
TrainJobQueue traingps_next_jobs(Train *train, TrainTrackStatus *status);

#endif /*__TRAIN_GPS_H__*/
