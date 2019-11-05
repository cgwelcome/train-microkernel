#include <string.h>
#include <server/name.h>
#include <user/ipc.h>
#include <user/name.h>
#include <utils/assert.h>

extern int name_server_tid;

int RegisterAs(const char *name) {
    assert(strlen(name) <= MAX_NAME_SIZE);

    NSRequest request;
    request.type = NS_REGISTER;
    strcpy(request.name, name);

    // TODO: block if name server is not ready
    if (name_server_tid < 0) {
        return -1;
    }

    return Send(name_server_tid, (char *)&request, sizeof(request), NULL, 0);
}

int WhoIs(const char *name) {
    assert(strlen(name) <= MAX_NAME_SIZE);

    NSRequest request;
    request.type = NS_WHOIS;
    strcpy(request.name, name);

    // TODO: block if name server is not ready
    if (name_server_tid < 0) {
        return -1;
    }

    int result;
    int error = Send(name_server_tid, (char *)&request, sizeof(request), (char *)&result, sizeof(result));
    if (error < 0) {
        return error;
    }
    return result;
}
