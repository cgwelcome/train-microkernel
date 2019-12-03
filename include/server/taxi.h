#ifndef __SERVER_TAXI_H__
#define __SERVER_TAXI_H__

#include <stdint.h>

typedef enum {
    TAXI_SUNRISE,
    TAXI_SUNSET,
    TAXI_CUSTOMER,
} TaxiNotification;

void CreateTaxiSystem();

#endif /*__SERVER_TAXI_H__*/
