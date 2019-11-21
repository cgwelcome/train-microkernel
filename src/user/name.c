#include <string.h>
#include <server/name.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>

static int InvokeNameServer() {
    register int ret asm("r0");
    SYSCALL_INVOKE(SYSCALL_NS_INVOKE);
    return ret;
}

void RegisterAs(const char *name) {
    assert(strlen(name) <= MAX_NAME_SIZE);

    NSRequest request;
    request.type = NS_REGISTER;
    strcpy(request.name, name);

    int name_server_tid = InvokeNameServer();
    assert(Send(name_server_tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

int WhoIs(const char *name) {
    assert(strlen(name) <= MAX_NAME_SIZE);

    NSRequest request;
    request.type = NS_WHOIS;
    strcpy(request.name, name);

    int result;
    int name_server_tid = InvokeNameServer();
    assert(Send(name_server_tid, (char *)&request, sizeof(request), (char *)&result, sizeof(result)) >= 0);
    return result;
}
