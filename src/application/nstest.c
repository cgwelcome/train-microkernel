#include <utils/kassert.h>
#include <user/ns.h>
#include <user/tasks.h>
#include <server/ns.h>

void register_test() {
    RegisterAs("Winnie");
    int tid = WhoIs("Winnie");
    kassert(tid == MyTid());
    Exit();
}

void nstest_root_task() {
    CreateNS(2000);
    Create(500, &register_test);
    Exit();
}
