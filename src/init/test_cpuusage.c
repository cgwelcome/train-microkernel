#include <user/tasks.h>
#include <utils/bwio.h>

#define A_PRIORITY   500
#define A_TEST_UNTIL 100000
#define B_PRIORITY   500
#define B_TEST_UNTIL 1000

void cpu_usage_test_worker_a() {
    bwprintf(COM2, "\033[1;1HTask A CPU Usage: ");
    for (int i = 0; i < A_TEST_UNTIL; i++) {
        bwprintf(COM2, "\033[1;19H\033[K%02d%%", MyCpuUsage());
    }
    bwprintf(COM2, "\033[1;1H\033[KTask A Finished.\033[3;1H");
    Exit();
}

void cpu_usage_test_worker_b() {
    bwprintf(COM2, "\033[2;1HTask B CPU Usage: ");
    for (int i = 0; i < B_TEST_UNTIL; i++) {
        bwprintf(COM2, "\033[2;19H\033[K%02d%%", MyCpuUsage());
    }
    bwprintf(COM2, "\033[2;1H\033[KTask B Finished.\033[3;1H");
    Exit();
}

void cpu_usage_test_root_task() {
    bwprintf(COM2, "\033[2J\033[?25l");
    Create(A_PRIORITY, &cpu_usage_test_worker_a);
    Create(B_PRIORITY, &cpu_usage_test_worker_b);
    Exit();
}
