#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <server/clock.h>
#include <server/trainset.h>
#include <server/shell.h>
#include <user/io.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/trainset.h>

void k4_root_task() {
    CreateNameServer();
    CreateClockServer();
    CreateIOServer();
    CreateTrainSetServer();
    CreateShellServer();
    CreateIdleTask();
    Exit();
}
