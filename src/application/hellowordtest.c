#include <ts7200.h>
#include <application.h>
#include <user/tasks.h>
#include <utils/bwio.h>

void hellowordtest_task() {
    bwprintf(COM2, "Hello world\n\r");
    Exit();
}
