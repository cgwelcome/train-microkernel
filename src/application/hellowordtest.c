#include <ts7200.h>
#include <application.h>
#include <user/tasks.h>
#include <utils/bwio.h>

void hellowordtest_root_task() {
    bwprintf(COM2, "Hello world\n\r");
    bwprintf(COM2, "%d\n\r", MyTid());
    bwprintf(COM2, "%d\n\r", MyParentTid());
    Exit();
}
