#include <train/driver.h>
#include <train/controller.h>
#include <train/manager.h>
#include <utils/assert.h>

static struct {
    TrainState state;
    void (*driver_handle)(Train *);
    void (*entry)(Train *);
} train_transition[] = {
    { TRAIN_STATE_CRUISE,        driver_cruise,        driver_speed_entry   },
    { TRAIN_STATE_BRAKE_COMMAND, driver_brake_command, driver_speed_entry   },
    { TRAIN_STATE_BRAKE_REVERSE, driver_brake_reverse, driver_speed_entry   },
    { TRAIN_STATE_BRAKE_TRAFFIC, driver_brake_traffic, driver_speed_entry   },
    { TRAIN_STATE_WAIT_COMMAND,  driver_wait_command,  driver_wait_entry    },
    { TRAIN_STATE_WAIT_REVERSE,  driver_wait_reverse,  driver_reverse_entry },
    { TRAIN_STATE_WAIT_TRAFFIC,  driver_wait_traffic , NULL                 },
    { TRAIN_STATE_NONE,          NULL,                 NULL                 },
};


static void driver_transition(Train *train, TrainState state) {
    for (uint32_t i = 0; train_transition[i].state != TRAIN_STATE_NONE; i++) {
        if (train_transition[i].state == state) {
            train->state = train_transition[i].state;
            train->driver_handle = train_transition[i].driver_handle;
            if (train_transition[i].entry != NULL) {
                train_transition[i].entry(train);
            }
            return;
        }
    }
    throw("unknown train state");
}

void driver_handle_speed(Train *train, uint32_t speed) {
    switch (train->state) {
        case TRAIN_STATE_CRUISE:
        case TRAIN_STATE_BRAKE_COMMAND:
        case TRAIN_STATE_BRAKE_REVERSE:
        case TRAIN_STATE_WAIT_COMMAND:
            train->speed = speed;
            if (speed == 0) {
                driver_transition(train, TRAIN_STATE_BRAKE_COMMAND);
            }
            else {
                driver_transition(train, TRAIN_STATE_CRUISE);
            }
            break;
        case TRAIN_STATE_BRAKE_TRAFFIC:
        case TRAIN_STATE_WAIT_REVERSE:
        case TRAIN_STATE_WAIT_TRAFFIC:
            train->original_speed = speed;
            break;
        case TRAIN_STATE_NONE:
            throw("unexpected train state none");
            break;
    }
}

void driver_handle_reverse(Train *train) {
    switch (train->state) {
        case TRAIN_STATE_CRUISE:
        case TRAIN_STATE_BRAKE_COMMAND:
            train->original_speed = train->speed;
            train->speed = 0;
            driver_transition(train, TRAIN_STATE_BRAKE_REVERSE);
            break;
        case TRAIN_STATE_BRAKE_REVERSE:
            break;
        case TRAIN_STATE_BRAKE_TRAFFIC:
            driver_transition(train, TRAIN_STATE_BRAKE_REVERSE);
            break;
        case TRAIN_STATE_WAIT_COMMAND:
        case TRAIN_STATE_WAIT_REVERSE:
        case TRAIN_STATE_WAIT_TRAFFIC:
            train->original_speed = train->speed;
            driver_transition(train, TRAIN_STATE_WAIT_REVERSE);
            break;
        case TRAIN_STATE_NONE:
            throw("unexpected train state none");
            break;
    }
}

void driver_handle_move(Train *train, uint32_t speed) {
    switch (train->state) {
        case TRAIN_STATE_CRUISE:
        case TRAIN_STATE_BRAKE_COMMAND:
        case TRAIN_STATE_WAIT_COMMAND:
            train->speed = speed;
            driver_transition(train, TRAIN_STATE_CRUISE);
            break;
        case TRAIN_STATE_BRAKE_REVERSE:
        case TRAIN_STATE_BRAKE_TRAFFIC:
        case TRAIN_STATE_WAIT_TRAFFIC:
        case TRAIN_STATE_WAIT_REVERSE:
            train->original_speed = speed;
            break;
        case TRAIN_STATE_NONE:
            throw("unexpected train state none");
            break;
    }
}

void driver_cruise(Train *train) {
    assert(train->speed > 0);
    if (train_manager_will_arrive_reverse(train)) {
        train->original_speed = train->speed;
        train->speed = 0;
        driver_transition(train, TRAIN_STATE_BRAKE_REVERSE);
    } else if (train_manager_will_arrive_final(train)) {
        train->speed = 0;
        driver_transition(train, TRAIN_STATE_BRAKE_COMMAND);
    } else if (train_manager_will_collide_train(train)) {
        train->original_speed = train->speed;
        train->speed = 0;
        driver_transition(train, TRAIN_STATE_BRAKE_TRAFFIC);
    }
}

void driver_brake_command(Train *train) {
    assert(train->speed == 0);
    if (train->velocity == 0) {
        driver_transition(train, TRAIN_STATE_WAIT_COMMAND);
    }
}

void driver_brake_reverse(Train *train) {
    assert(train->speed == 0);
    if (train->velocity == 0) {
        driver_transition(train, TRAIN_STATE_WAIT_REVERSE);
    }
}

void driver_brake_traffic(Train *train) {
    assert(train->speed == 0);
    if (train->velocity == 0) {
        driver_transition(train, TRAIN_STATE_WAIT_TRAFFIC);
    } else if (train_manager_unblocked_train(train)) {
        train->speed = train->original_speed;
        driver_transition(train, TRAIN_STATE_CRUISE);
    }
}

void driver_wait_command(Train *train) {
    assert(train->speed == 0);
    if (train_manager_will_arrive_reverse(train)) {
        driver_transition(train, TRAIN_STATE_WAIT_REVERSE);
    } else if (train_manager_will_collide_train(train)) {
        driver_transition(train, TRAIN_STATE_WAIT_TRAFFIC);
    }
}

void driver_wait_reverse(Train *train) {
    assert(train->speed == 0);
    if (train_manager_will_arrive_final(train)) {
        // Original speed is irrelevant since train has arrived at its destination
        driver_transition(train, TRAIN_STATE_WAIT_COMMAND);
    } else if (train_manager_will_collide_train(train)) {
        // Original speed is kept when stuck in traffic
        driver_transition(train, TRAIN_STATE_WAIT_TRAFFIC);
    } else if (train->original_speed > 0) {
        train->speed = train->original_speed;
        driver_transition(train, TRAIN_STATE_CRUISE);
    } else {
        driver_transition(train, TRAIN_STATE_WAIT_COMMAND);
    }
}

void driver_wait_traffic(Train *train) {
    assert(train->speed == 0);
    // Driver will reverse if train manager re-route the train
    if (train_manager_will_arrive_reverse(train)) {
        // Original speed is kept when reversing
        driver_transition(train, TRAIN_STATE_WAIT_REVERSE);
    } else if (train_manager_unblocked_train(train)) {
        train->speed = train->original_speed;
        if (train->speed > 0) {
            driver_transition(train, TRAIN_STATE_CRUISE);
        } else {
            driver_transition(train, TRAIN_STATE_WAIT_COMMAND);
        }
    }
}

void driver_wait_entry(Train *train) {
    position_clear(&train->final_position);
}

void driver_speed_entry(Train *train) {
    controller_speed_one(train->id, train->speed, 0);
}

void driver_reverse_entry(Train *train) {
    if (train->mode == TRAIN_MODE_PATH) {
        assert(train->reverse_anchor.node != NULL);
        path_next_node(&train->path, train->reverse_anchor.node->reverse);
        train_manager_setup_reverse(train);
    }
    train->position = position_reverse(train->position);
    controller_speed_one(train->id, TRAIN_STATUS_REVERSE, 0);
}
