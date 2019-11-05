#include <string.h>
#include <server/name.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>

extern int name_server_tid;

static int PingNameServer() {
    int retry = 0;
    while (name_server_tid < 0) {
        if (retry >= 500) return -1;
        retry++;
        Yield();
    }
    return 0;
}

void RegisterAs(const char *name) {
    assert(strlen(name) <= MAX_NAME_SIZE);

    NSRequest request;
    request.type = NS_REGISTER;
    strcpy(request.name, name);

    if (PingNameServer() < 0) {
        throw("RegisterAs failed, name server not ready");
    }

    assert(Send(name_server_tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

int WhoIs(const char *name) {
    assert(strlen(name) <= MAX_NAME_SIZE);

    NSRequest request;
    request.type = NS_WHOIS;
    strcpy(request.name, name);

    if (PingNameServer() < 0) {
        throw("WhoIs failed, name server not ready");
    }

    int result;
    assert(Send(name_server_tid, (char *)&request, sizeof(request), (char *)&result, sizeof(result)) >= 0);
    return result;
}
