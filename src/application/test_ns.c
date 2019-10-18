#include <server/name.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/kassert.h>

void register_test() {
    RegisterAs("Winnie");
    int tid = WhoIs("Winnie");
    kassert(tid == MyTid());
    Exit();
}

void ns_test_root_task() {
    CreateNameServer(2000);
    Create(500, &register_test);
    Exit();
}