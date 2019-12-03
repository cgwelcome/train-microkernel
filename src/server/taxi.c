#include <kernel.h>
#include <server/taxi.h>
#include <user/clock.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/train.h>
#include <utils/assert.h>

void taxi_system_root_task() {
    int tid;
    TaxiNotification notification;

    RegisterAs(SERVER_NAME_TAXI);
    int traintid = WhoIs(SERVER_NAME_TRAIN);
    for (;;) {
        Receive(&tid, (char *)&notification, sizeof(notification));
        Reply(tid, NULL, 0);
        switch (notification) {
            case TAXI_SUNRISE:
                TrainLight(traintid, false);
                break;
            case TAXI_SUNSET:
                TrainLight(traintid, true);
                break;
            case TAXI_CUSTOMER:
                break;
            default:
                throw("unknown request");
        }
    }
    Exit();
}

void taxi_system_notifier_suntime() {
    int clocktid = WhoIs(SERVER_NAME_CLOCK);
    int taxitid  = WhoIs(SERVER_NAME_TAXI);

    TaxiNotification suntime_notification = TAXI_SUNRISE;

    for (;;) {
        Delay(clocktid, 18000); // 3 minutes
        if (suntime_notification == TAXI_SUNRISE) {
            suntime_notification = TAXI_SUNSET;
        } else {
            suntime_notification = TAXI_SUNRISE;
        }
        assert(Send(taxitid, (char *)&suntime_notification, sizeof(suntime_notification), NULL, 0) >= 0);
    }
    Exit();
}

void taxi_system_notifier_customer() {
    int clocktid = WhoIs(SERVER_NAME_CLOCK);
    int taxitid  = WhoIs(SERVER_NAME_TAXI);

    TaxiNotification customer_notification = TAXI_CUSTOMER;

    for (;;) {
        Delay(clocktid, 500); // 5 seconds
        assert(Send(taxitid, (char *)&customer_notification, sizeof(customer_notification), NULL, 0) >= 0);
    }
    Exit();
}

void CreateTaxiSystem() {
    Create(PRIORITY_SERVER_TAXI, &taxi_system_root_task);
    Create(PRIORITY_NOTIFIER_TAXI_SUNTIME,  &taxi_system_notifier_suntime);
    Create(PRIORITY_NOTIFIER_TAXI_CUSTOMER, &taxi_system_notifier_customer);
}
