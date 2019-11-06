#ifndef __TRAIN_GPS_H__
#define __TRAIN_GPS_H__

#include <train/job.h>
#include <train/manager.h>

/**
 * Find the path from src to destination based on traffic
 */
TrainPath traingps_find(TrainTrackEdge *src, TrainTrackEdge *dest, TrainTrackStatus *status);

/**
 * Check wheter a position is on top of a sensor
 */
uint32_t traingps_is_sensor(TrainTrackEdge *position);

/**
 * Make self-referencing edge to node with offset zero
 */
TrainTrackEdge traingps_node_to_edge(TrainTrackNode *node);

/**
 * Convert a node to sensor
 */
TrainSensor traingps_node_to_sensor(TrainTrackNode *node);

/**
 * Convert from a node to a switch id
 */
uint32_t traingps_node_to_switch(TrainTrackNode *node);

/**
 * Get the current positive as the next relative distance,
 * if the path has no more sensors, then postion base node is NODE_NONE
 */
TrainTrackEdge traingps_next_dest(TrainPath *path);

/**
 * Return any train job that is need to be schedule, i.e. stop a train after
 * a time delay
 */
TrainJobQueue traingps_next_jobs(Train *train, TrainTrackStatus *status);

/**
 * Update the train next position, and the Estimated time of clock
 * to the next dest. The time depends on the train position, velocity,
 * acceleration, any other interferences from other trains, and track calibration
 */
void traingps_update_next(Train *train, uint32_t time, TrainTrackStatus *status);

#endif /*__TRAIN_GPS_H__*/
