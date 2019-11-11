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

/**
 * Initialize Train Manager
 */
void trainmanager_init();
/**
 * Initialize Train Tracks
 */
void trainmanager_init_track(TrackName name);
/**
 * Set initial position of the train
 */
void trainmanager_start(uint32_t train_id);
/**
 * Set train to speed
 */
void trainmanager_speed(uint32_t train_id, uint32_t speed);
/**
 * Move train to a node_id with a positive offset
 * running at speed
 */
// void trainmanager_move(uint32_t train_id, uint32_t speed, uint32_t node_id, uint32_t offset);
/**
 * Reverse Train with same speed
 */
void trainmanager_reverse(uint32_t train_id);
/**
 * Switch all switches
 */
void trainmanager_switch_all(int8_t status);
/**
 * Switch one switch, TODO: Combine Switchone and Switchall
 */
void trainmanager_switch_one(uint32_t switch_id, int8_t status);
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
