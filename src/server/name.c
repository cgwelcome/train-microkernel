#include <string.h>
#include <server/name.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>

typedef struct {
    char name[MAX_NAME_SIZE];
    int tid;
} NameRecord;

int name_server_tid;
static int record_count;
static NameRecord records[MAX_NAMERECORD_NUM];

static int name_search(const char *name) {
    for (int i = 0; i < record_count; i++) {
        if (strcmp(records[i].name, name) == 0) {
            return i; // Found a registration
        }
    }
    return -1; // Not found
}

static int name_whois(const char *name) {
    int i = name_search(name);
    if (i == -1) return -1;
    return records[i].tid;
}

static int name_register(int tid, const char *name) {
    if (record_count == MAX_NAMERECORD_NUM) {
        return -1; // Reached the limit of registration
    }
    // Search for existing registration
    int i = name_search(name);
    if (i != -1) {
        records[i].tid = tid;
        return 0;
    }
    // Add name registration
    strcpy(records[record_count].name, name);
    records[record_count].tid = tid;
    record_count++;
    return 0;
}

static void name_server_task() {
    int retval;
    int tid;
    NSRequest request;

    for (;;) {
        // Wait for a name request or lookup
        Receive(&tid, (char *)&request, sizeof(request));
        switch (request.type) {
            case NS_REGISTER:
                retval = name_register(tid, request.name);
                break;
            case NS_WHOIS:
                retval = name_whois(request.name);
                break;
            default:
                retval = -1;
                break;
        }
        // Reply with the appropriate value
        Reply(tid, (char *)&retval, sizeof(retval));
    }
}

void InitNameServer() {
    name_server_tid = -1;
    record_count = 0;
}

int CreateNameServer(uint32_t priority) {
    if (name_server_tid < 0) {
        name_server_tid = Create(priority, &name_server_task);
    }
    return name_server_tid;
}
