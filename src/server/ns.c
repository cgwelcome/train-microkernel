#include <string.h>
#include <server/ns.h>
#include <user/ipc.h>
#include <user/ns.h>
#include <user/tasks.h>

typedef struct {
    char name[MAX_NAME_SIZE];
    int tid;
} NameRecord;

static unsigned int record_count;
static NameRecord records[MAX_NAMERECORD_NUM];

static int ns_whois(const char *name) {
    for (unsigned int i = 0; i < record_count; i++) {
        if (strcmp(records[i].name, name) == 0) {
            return records[i].tid; // Found a registration
        }
    }
    return -1; // Not found
}

static int ns_register(int tid, const char *name) {
    for (unsigned int i = 0; i < record_count; i++) {
        if (!strcmp(records[i].name, name)) {
            records[i].tid = tid; // Found a registration, overwrite
            return 0;
        }
    }
    if (record_count == MAX_NAMERECORD_NUM) {
        return -1; // Reached the limit of registration
    }
    // Add name registration
    strcpy(records[record_count].name, name);
    records[record_count].tid = tid;
    record_count++;
    return 0;
}

static void ns_task() {
    int retval;
    int tid;
    NSRequest request;

    // Initialize Name server
    record_count = 0;
    ns_tid = MyTid();
    for (;;) {
        // Wait for a name request or lookup
        Receive(&tid, (char *)&request, sizeof(request));
        switch (request.type) {
            case NS_REGISTER:
                retval = ns_register(tid, request.name);
                break;
            case NS_WHOIS:
                retval = ns_whois(request.name);
                break;
            default:
                retval = -1;
                break;
        }
        // Reply with the appropriate value
        Reply(tid, (char *)&retval, sizeof(retval));
    }
}

int CreateNS(unsigned int priority) {
    return Create(priority, &ns_task);
}
