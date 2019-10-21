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
    CreateNameServer(4000);
    CreateClockServer(3700);
    CreateIOServer(3500, 3500, 3500);
    CreateTrainSetServer(3000);
    CreateShellServer(2000);
    CreateIdleTask(1);
    Exit();
}
