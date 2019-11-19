#include <hardware/timer.h>
#include <train/model.h>
#include <utils/assert.h>

uint32_t acceleration_table[5] = { 137, 132, 124, 195, 125 };

uint32_t expected_velocity_matrix[5][5] = {
    {340, 400, 470, 530, 600},
    {350, 410, 480, 550, 630},
    {320, 380, 450, 520, 600},
    {490, 550, 600, 630, 630},
    {270, 330, 390, 440, 500},
};

static uint32_t expected_acceleration(uint32_t train_id) {
    return acceleration_table[train_id_to_index(train_id)];
}

static uint32_t expected_velocity(uint32_t train_id, uint32_t speed) {
    if (speed == 0) return 0;
    assert(speed >= 10 && speed <= 14);
    return expected_velocity_matrix[train_id_to_index(train_id)][speed - 10];
}

static uint32_t model_integrate_velocity(uint32_t current_vec, uint32_t target_vec, uint32_t acc, uint32_t dt) {
    if (current_vec == target_vec) return current_vec;

    uint32_t dv = acc * dt / 1000;
    if (current_vec < target_vec) {
        uint32_t remain = target_vec - current_vec;
        return current_vec + (dv < remain ? dv : remain);
    } else {
        uint32_t remain = current_vec - target_vec;
        return current_vec - (dv < remain ? dv : remain);
    }
}

void model_estimate_train_status(Train *train) {
    assert(train->inited);

    uint32_t now = (uint32_t) timer_read(TIMER3);
    uint32_t dt = now - train->last_position_update_time;
    train->last_position_update_time = now;

    uint32_t acc = expected_acceleration(train->id);
    uint32_t vec = expected_velocity(train->id, train->speed);

    uint32_t v0 = train->velocity;
    train->velocity = model_integrate_velocity(v0, vec, acc, dt);
    uint32_t vt = train->velocity;
    uint32_t dd = (v0 + vt) * dt / 2;
    if (train->position.node != NULL) {
        position_move(&train->position, (int32_t) dd);
    }
}

uint32_t model_estimate_train_stop_distance(Train *train) {
    assert(train->inited);
    uint32_t acc = expected_acceleration(train->id);
    uint32_t vec = train->velocity;
    return (vec * vec) / (2 * acc);
}
