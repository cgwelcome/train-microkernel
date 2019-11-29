#include <train/driver.h>
#include <train/controller.h>
#include <train/manager.h>
#include <utils/assert.h>

static struct {
    TrainState state;
    void (*handle)(Train *, DriverRequest *);
    void (*entry)(Train *);
} train_transition[] = {
    { TRAIN_STATE_CRUISE,        driver_cruise,        driver_speed_entry   },
    { TRAIN_STATE_BRAKE_COMMAND, driver_brake_command, driver_speed_entry   },
    { TRAIN_STATE_BRAKE_REVERSE, driver_brake_reverse, driver_speed_entry   },
    { TRAIN_STATE_BRAKE_TRAFFIC, driver_brake_traffic, driver_speed_entry   },
    { TRAIN_STATE_WAIT_COMMAND,  driver_wait_command,  NULL                 },
    { TRAIN_STATE_WAIT_REVERSE,  driver_wait_reverse,  driver_reverse_entry },
    { TRAIN_STATE_WAIT_TRAFFIC,  driver_wait_traffic , NULL                 },
    { TRAIN_STATE_NONE,          NULL,                 NULL                 },
};

void driver_transition(Train *train, TrainState state) {
    for (uint32_t i = 0; train_transition[i].state != TRAIN_STATE_NONE; i++) {
        if (train_transition[i].state == state) {
            train->state = state;
            train_transition[i].entry(train);
            return;
        }
    }
    throw("Unknown train state");
}

void driver_cruise(Train *train, DriverRequest *request) {
    assert(train->speed > 0);
    if (train_manager_will_arrive(train)) {
        train->speed = 0;
        driver_transition(train, TRAIN_STATE_BRAKE_COMMAND);
    }
    else if (train_manager_will_reverse(train)) {
        train->original_speed = train->speed;
        train->speed = 0;
        driver_transition(train, TRAIN_STATE_BRAKE_REVERSE);
    }
    else if (train_manager_will_collide(train)) {
        train->speed = 0;
        driver_transition(train, TRAIN_STATE_BRAKE_TRAFFIC);
    }
    else if (request != NULL) {
        switch (request->type) {
            case DRIVER_REQUEST_SPEED:
                train->speed = request->speed;
                if (train->speed > 0) {
                    driver_transition(train, TRAIN_STATE_CRUISE);
                }
                else {
                    driver_transition(train, TRAIN_STATE_BRAKE_COMMAND);
                }
                break;
            case DRIVER_REQUEST_REVERSE:
                train->original_speed = train->speed;
                train->speed = 0;
                driver_transition(train, TRAIN_STATE_BRAKE_REVERSE);
                break;
        }
    }
}

void driver_brake_command(Train *train, DriverRequest *request) {
    (void)request;
    assert(train->speed == 0);
    if (train->velocity == 0) {
        driver_transition(train, TRAIN_STATE_WAIT_COMMAND);
    }
}

void driver_brake_reverse(Train *train, DriverRequest *request) {
    (void)request;
    assert(train->speed == 0);
    if (train->velocity == 0) {
        driver_transition(train, TRAIN_STATE_WAIT_REVERSE);
    }
}

void driver_brake_traffic(Train *train, DriverRequest *request) {
    (void)request;
    assert(train->speed == 0);
    if (train->velocity == 0) {
        driver_transition(train, TRAIN_STATE_WAIT_TRAFFIC);
    }
    else if (train_manager_is_unblocked(train)) {
        train->speed = train->original_speed;
        driver_transition(train, TRAIN_STATE_CRUISE);
    }
}

void driver_wait_command(Train *train, DriverRequest *request) {
    assert(train->speed == 0);
    if (train_manager_will_reverse(train)) {
        driver_transition(train, TRAIN_STATE_WAIT_REVERSE);
    }
    else if (train_manager_will_collide(train)) {
        driver_transition(train, TRAIN_STATE_WAIT_TRAFFIC);
    }
    else if (request != NULL) {
        switch (request->type) {
            case DRIVER_REQUEST_SPEED:
                train->speed = request->speed;
                if (train->speed > 0) {
                    driver_transition(train, TRAIN_STATE_CRUISE);
                }
                break;
            case DRIVER_REQUEST_REVERSE:
                train->original_speed = train->speed;
                driver_transition(train, TRAIN_STATE_WAIT_REVERSE);
                break;
        }
    }
}

void driver_wait_reverse(Train *train, DriverRequest *request) {
    (void)request;
    assert(train->speed == 0);
    if (train_manager_will_arrive(train)) {
        driver_transition(train, TRAIN_STATE_WAIT_COMMAND);
    }
    else if (train_manager_will_collide(train)) {
        driver_transition(train, TRAIN_STATE_WAIT_TRAFFIC);
    }
    else if (train->original_speed > 0) {
        train->speed = train->original_speed;
        driver_transition(train, TRAIN_STATE_CRUISE);
    }
    else {
        driver_transition(train, TRAIN_STATE_WAIT_COMMAND);
    }
}

void driver_wait_traffic(Train *train, DriverRequest *request) {
    assert(train->speed == 0);
    if (train_manager_is_unblocked(train)) {
        driver_transition(train, TRAIN_STATE_WAIT_COMMAND);
    }
    else if (request != NULL) {
        switch (request->type) {
            case DRIVER_REQUEST_SPEED:
                break;
            case DRIVER_REQUEST_REVERSE:
                driver_transition(train, TRAIN_STATE_WAIT_REVERSE);
                break;
        }
    }
}

void driver_speed_entry(Train *train) {
    controller_speed_one(train->id, train->speed, 0);
}

void driver_reverse_entry(Train *train) {
    controller_speed_one(train->id, TRAIN_STATUS_REVERSE, 0);
}
