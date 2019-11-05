#ifndef __TRAIN_MANAGER_H__
#define __TRAIN_MANAGER_H__

#include <stdint.h>
#include <server/trainmanager.h>
#include <train/job.h>
#include <train/track.h>
#include <train/trainset.h>

// Highest MAX_{}_NUM means highest id
#define MAX_TRAIN_NUM              80
#define MAX_SWITCH_NUM            157
#define TRAINSWITCH_DONE_INTERVAL  15 /** Interval in ticks (10 ms) */

typedef struct {
    uint32_t id;
	TrainSwitchStatus status;
} TrainSwitch;

typedef enum {
	TRAINMODE_FREE, /** Train roaming without a destination */
	TRAINMODE_PATH, /** Train with a specific path */
} TrainMode;

typedef struct {
	TrainTrackNode base;
	int32_t offset; /** offset in mm from the base node */
} TrainPosition;

typedef struct {
	TrainTrackNode nodes[MAX_NODE_PER_TRACK];
	uint32_t index; /** Edge >= index that has not yet been visited by train */
	uint32_t size;
	TrainPosition dest; /** Destination node is usually the last node of path, with offset */
} TrainPath;

typedef struct {
    uint32_t id;
	TrainMode mode;
    uint32_t speed;
	TrainPath path;
	TrainPosition last_position; /** Last seen position */
	TrainPosition next_position; /** Next position */
	uint64_t next_time; /** Estimated Arrival Time to Next dest */
} Train;

typedef struct {
	Train trains[MAX_TRAIN_NUM];
	TrainSwitch trainswitches[MAX_SWITCH_NUM];
	TrainTrack track;
	TrainJobQueue jobqueue;
} TrainTrackStatus;

/**
 * Initialize Train Manager
 */
void trainmanager_init();
/**
 * Initialize Train Tracks
 */
void trainmanager_init_track(TrainTrackType type);
/**
 * Set train to speed
 */
void trainmanager_speed(uint32_t train_id, uint32_t speed);
/**
 * Move train to a node_id with a positive or negative offset
 * running at speed
 */
void trainmanager_move(uint32_t train_id, uint32_t speed, uint32_t node_id, int32_t offset);
/**
 * Reverse Train with same speed
 */
void trainmanager_reverse(uint32_t train_id);
/**
 * Switch all switches
 */
void trainmanager_switch_all(TrainSwitchStatus status);
/**
 * Switch one switch, TODO: Combine Switchone and Switchall
 */
void trainmanager_switch_one(uint32_t switch_id, TrainSwitchStatus status);
/**
 * Turn off switch after an interval
 */
void trainmanager_switch_done();
/**
 * Update the status of Train Manager
 */
void trainmanager_update_status();
/**
 * Respond with a job to be scheduled
 */
void trainmanager_init_job(int32_t tid);
/**
 * Notify manager the train has fully stopped
 */
void trainmanager_park(uint32_t train_id);
/**
 * Emergency stop the train
 */
void trainmanager_stop();
/**
 * Stop all train when done
 */
void trainmanager_done();
/**
 * Hash a trainsensor object
 */
uint32_t trainsensor_hash(TrainSensor *sensor);

#endif /*__TRAIN_MANAGER_H__*/
