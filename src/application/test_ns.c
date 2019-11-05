#include <server/name.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>

void register_test() {
    RegisterAs("Winnie");
    int tid = WhoIs("Winnie");
    assert(tid == MyTid());
    Exit();
}

void ns_test_root_task() {
    CreateNameServer();
    Create(500, &register_test);
    Exit();
}
