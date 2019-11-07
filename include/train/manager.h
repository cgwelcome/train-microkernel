#ifndef __TRAIN_MANAGER_H__
#define __TRAIN_MANAGER_H__

#include <stdint.h>
#include <server/train.h>
#include <train/track.h>
#include <train/trainset.h>
#include <utils/queue.h>

// Highest MAX_{}_NUM means highest id
#define MAX_TRAIN_NUM              80
#define MAX_SWITCH_NUM            157
#define TRAINSWITCH_DONE_INTERVAL  15 /** Interval in ticks (10 ms) */
#define MAX_SPEED_NUM              15

#define DEFAULTSWITCH_STATUS TRAIN_SWITCH_CURVED

typedef struct {
    uint32_t id;
    TrainSwitchStatus status;
} TrainSwitch;

typedef enum {
    TRAINMODE_FREE, /** Train roaming without a destination */
    TRAINMODE_PATH, /** Train with a specific path */
} TrainMode;

typedef struct {
    TrainTrackNode *nodes[MAX_NODE_PER_TRACK]; /** Nodes to be visited, reference to track */
    uint32_t size;
    uint32_t index; /** Node >= index that has not yet been visited by train */
	TrainTrackEdge src;
	TrainTrackEdge dest;
} TrainPath;

typedef struct {
    uint32_t id;
    TrainMode mode;
    uint32_t speed;
    TrainPath path;
    uint32_t active; /** Activate */
    TrainTrackEdge last_position; /** Pointer to track of last position seen position */
    TrainTrackEdge next_position; /** Pointer to track of next position */
    uint64_t next_time; /** Estimated Arrival Time to Next dest */
} Train;

typedef struct {
    Train trains[MAX_TRAIN_NUM];
    TrainSwitch trainswitches[MAX_SWITCH_NUM];
    TrainTrack track;
	Queue awaitsensors[MAX_SENSOR_NUM];
	Queue initialtrains;
	uint32_t velocities[MAX_SPEED_NUM]; /** Velocity, ticks/mm */
} TrainTrackStatus;

/**
 * Initialize Train Manager
 */
void trainmanager_init();
/**
 * Initialize Train Tracks
 */
void trainmanager_init_track(TrainTrackName name);
/**
 * Set train to speed
 */
void trainmanager_speed(uint32_t train_id, uint32_t speed);
/**
 * Move train to a node_id with a positive offset
 * running at speed
 */
void trainmanager_move(uint32_t train_id, uint32_t speed, uint32_t node_id, uint32_t offset);
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
 * Notify manager the train has fully stopped
 */
void trainmanager_park(uint32_t train_id);
/**
 * Manually set a speed with constant velocity
 */
void trainmanager_set_velocity(uint32_t speed, uint32_t velocity);
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
