#include <server/name.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>
#include <utils/bwio.h>

void whois_block_test() {
    bwprintf(COM2, "Block on WhoIs()\r\n");
    int tid = WhoIs("Winnie");
    bwprintf(COM2, "Winnie is %d\r\n", tid);
    Exit();
}

void register_test() {
    RegisterAs("Winnie");
    bwprintf(COM2, "Task %d registered as Winnie\r\n", MyTid());
    Exit();
}

void ns_test_root_task() {
    CreateNameServer();
    Create(1000, &whois_block_test);
    for (int i = 0; i < 100; i++) Yield();
    Create(1000, &register_test);
    Exit();
}
