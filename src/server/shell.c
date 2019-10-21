#include <user/tasks.h>

static void shell_parser_task() {
    Exit();
}

static void shell_keyboard_task() {
    Exit();
}

static void shell_clock_task() {
    Exit();
}

static void shell_sensor_task() {
    Exit();
}

void CreateShellServer(uint32_t priority) {
    Create(priority, &shell_parser_task);
    Create(priority, &shell_keyboard_task);
    Create(priority, &shell_clock_task);
    Create(priority, &shell_sensor_task);
}
