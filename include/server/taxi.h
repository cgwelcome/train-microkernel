#ifndef __SERVER_TAXI_H__
#define __SERVER_TAXI_H__

#include <train/track.h>

#include <stdint.h>

#define TAXI_TRIP_POOL_SIZE 8

typedef enum {
    TAXI_NOTIFY_SUNRISE,
    TAXI_NOTIFY_SUNSET,
    TAXI_NOTIFY_CUSTOMER,
} TaxiNotification;

typedef enum {
    TAXI_STATE_IDLE,
    TAXI_STATE_PICKING,
    TAXI_STATE_DROPPING,
} TaxiState;

typedef struct {
    char *   pickup_node;
    char *   dropoff_node;
} TaxiTripCandidate;

typedef struct {
    TaxiState state;
    TrackNode *pickup_node;
    TrackNode *dropoff_node;
} TaxiRecord;

void CreateTaxiSystem();

#endif /*__SERVER_TAXI_H__*/
