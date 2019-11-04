#include <application.h>
#include <user/tasks.h>
#include <utils/bwio.h>

void helloworld_test_root_task() {
    bwprintf(COM2, "Hello world\r\n");
    bwprintf(COM2, "%d\r\n", MyTid());
    bwprintf(COM2, "%d\r\n", MyParentTid());
    Exit();
}
