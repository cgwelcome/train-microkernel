#include <string.h>
#include <user/ns.h>
#include <user/ipc.h>
#include <server/ns.h>

extern int ns_tid;

int RegisterAs(const char *name) {
    int result;
    NSRequest request;

    // Name too long to be stored
    if (strlen(name) > MAX_NAME_SIZE) {
        return -2;
    }
    request.type = NS_REGISTER;
    strcpy(request.name, name);
    int retval = Send(ns_tid, (char *)&request, sizeof(request),
            (char *)&result, sizeof(result));
    if (retval == -1) {
        return -1;
    }
    return result;
}

int WhoIs(const char *name) {
    int result;
    NSRequest request;

    // Name too long to be stored
    if (strlen(name) > MAX_NAME_SIZE) {
        return -2;
    }
    request.type = NS_WHOIS;
    strcpy(request.name, name);
    int retval = Send(ns_tid, (char *)&request, sizeof(request),
            (char *)&result, sizeof(result));
    if (retval == -1) {
        return -1;
    }
    return result;
}
