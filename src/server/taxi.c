#include <kernel.h>
#include <hardware/timer.h>
#include <server/taxi.h>
#include <train/train.h>
#include <user/clock.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/train.h>
#include <utils/assert.h>

extern Track singleton_track;

static TaxiRecord records[TRAIN_COUNT];
static const TaxiTripCandidate trip_pool[TAXI_TRIP_POOL_SIZE] = {
    {"B15", "D16"},
    {"C9" , "E7" },
    {"C5" , "D13"},
    {"D1" , "D11"},
    {"E5" , "E2" },
    {"C13", "E15"},
    {"D11", "E16"},
    {"E5" , "A3" },
};

static void taxi_schedule_trip(TaxiRecord *record) {
    uint64_t now = timer_read_raw(TIMER3);
    const TaxiTripCandidate *candidate = &trip_pool[now % TAXI_TRIP_POOL_SIZE];
    record->pickup_node  = track_find_node_by_name(&singleton_track, candidate->pickup_node);
    record->dropoff_node = track_find_node_by_name(&singleton_track, candidate->dropoff_node);
}

static bool taxi_schedule_has_conflict(TaxiRecord *record) {
    for (uint32_t i = 0; i < TRAIN_COUNT; i++) {
        if (records + i == record) continue;
        if (records[i].state == TAXI_STATE_IDLE) continue;
        if (records[i].pickup_node  == record->pickup_node)  return true;
        if (records[i].pickup_node  == record->dropoff_node) return true;
        if (records[i].dropoff_node == record->pickup_node)  return true;
        if (records[i].dropoff_node == record->dropoff_node) return true;
    }
    return false;
}

static void taxi_check_next_step(int traintid, uint32_t idle_train) {
    TaxiRecord *record = &records[train_id_to_index(idle_train)];
    if (record->state == TAXI_STATE_IDLE) {
        do {
            taxi_schedule_trip(record);
        } while(taxi_schedule_has_conflict(record));
    }
    switch (record->state) {
        case TAXI_STATE_IDLE:
            TrainMove(traintid, idle_train, 10, record->pickup_node, 0);
            record->state = TAXI_STATE_PICKING;
            break;
        case TAXI_STATE_PICKING:
            TrainMove(traintid, idle_train, 10, record->dropoff_node, 0);
            record->state = TAXI_STATE_DROPPING;
            break;
        case TAXI_STATE_DROPPING:
            record->state = TAXI_STATE_IDLE;
            break;
        default:
            throw("unknown state");
    }
}

static void taxi_system_root_task() {
    int tid;
    TaxiNotification notification;

    RegisterAs(SERVER_NAME_TAXI);
    int traintid = WhoIs(SERVER_NAME_TRAIN);
    for (int i = 0; i < TRAIN_COUNT; i++) {
        records[i].state = TAXI_STATE_IDLE;
    }
    for (;;) {
        Receive(&tid, (char *)&notification, sizeof(notification));
        Reply(tid, NULL, 0);
        switch (notification) {
            case TAXI_NOTIFY_SUNRISE:
                TrainLight(traintid, false);
                break;
            case TAXI_NOTIFY_SUNSET:
                TrainLight(traintid, true);
                break;
            case TAXI_NOTIFY_CUSTOMER:
                if (singleton_track.inited) {
                    uint32_t idle_train = TrainFindIdle(traintid);
                    while (idle_train != 0) {
                        taxi_check_next_step(traintid, idle_train);
                        idle_train = TrainFindIdle(traintid);
                    }
                }
                break;
            default:
                throw("unknown request");
        }
    }
    Exit();
}

static void taxi_system_notifier_suntime() {
    int clocktid = WhoIs(SERVER_NAME_CLOCK);
    int taxitid  = WhoIs(SERVER_NAME_TAXI);

    TaxiNotification suntime_notification = TAXI_NOTIFY_SUNRISE;

    for (;;) {
        Delay(clocktid, 6000); // 60 seconds
        if (suntime_notification == TAXI_NOTIFY_SUNRISE) {
            suntime_notification = TAXI_NOTIFY_SUNSET;
        } else {
            suntime_notification = TAXI_NOTIFY_SUNRISE;
        }
        assert(Send(taxitid, (char *)&suntime_notification, sizeof(suntime_notification), NULL, 0) >= 0);
    }
    Exit();
}

static void taxi_system_notifier_customer() {
    int clocktid = WhoIs(SERVER_NAME_CLOCK);
    int taxitid  = WhoIs(SERVER_NAME_TAXI);

    TaxiNotification customer_notification = TAXI_NOTIFY_CUSTOMER;

    for (;;) {
        Delay(clocktid, 1000); // 10 seconds
        assert(Send(taxitid, (char *)&customer_notification, sizeof(customer_notification), NULL, 0) >= 0);
    }
    Exit();
}

void CreateTaxiSystem() {
    Create(PRIORITY_SERVER_TAXI, &taxi_system_root_task);
    Create(PRIORITY_NOTIFIER_TAXI_SUNTIME,  &taxi_system_notifier_suntime);
    Create(PRIORITY_NOTIFIER_TAXI_CUSTOMER, &taxi_system_notifier_customer);
}
