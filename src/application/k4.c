#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <server/clock.h>
#include <server/train.h>
#include <server/shell.h>
#include <user/tasks.h>

void k4_root_task() {
    CreateNameServer();
    CreateClockServer();
    CreateIOServer();
    CreateTrainManagerServer();
    CreateShellServer();
    CreateIdleTask();
    Exit();
}
