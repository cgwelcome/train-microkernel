#include <string.h>
#include <kernel.h>
#include <server/name.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/assert.h>
#include <utils/queue.h>

typedef struct {
    char name[MAX_NAME_SIZE];
    int tid;
} NameRecord;

static Queue whois_queue;
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

static void name_register(int tid, const char *name) {
    // Search for existing registration
    int i = name_search(name);
    if (i != -1) {
        records[i].tid = tid;
        return;
    }
    // Add name registration
    assert(record_count < MAX_NAMERECORD_NUM);
    strcpy(records[record_count].name, name);
    records[record_count].tid = tid;
    record_count++;
}

static void name_flush_whois_queue() {
    NSRequest request;
    for (size_t i = 0; i < queue_size(&whois_queue); i++) {
        int *whois_tid = queue_at(&whois_queue, i);
        if (*whois_tid != -1) {
            Peek(*whois_tid, (char *)&request, sizeof(request));
            assert(request.type == NS_WHOIS);
            int tid = name_whois(request.name);
            if (tid >= 0) {
                Reply(*whois_tid, (char *)&tid, sizeof(tid));
                *whois_tid = -1;
            }
        }
    }
    while (queue_size(&whois_queue) > 0 && queue_peek(&whois_queue) == -1) {
        queue_pop(&whois_queue);
    }
}

void name_server_root_task() {
    int retval;
    int tid;
    NSRequest request;

    record_count = 0;
    queue_init(&whois_queue);
    for (;;) {
        // Wait for a name request or lookup
        Receive(&tid, (char *)&request, sizeof(request));
        switch (request.type) {
            case NS_REGISTER:
                name_register(tid, request.name);
                Reply(tid, NULL, 0);
                name_flush_whois_queue();
                break;
            case NS_WHOIS:
                retval = name_whois(request.name);
                if (retval < 0) {
                    queue_push(&whois_queue, tid);
                } else {
                    Reply(tid, (char *)&retval, sizeof(retval));
                }
                break;
            default:
                throw("unknown request");
        }
    }
}
