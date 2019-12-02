#include <ctype.h>
#include <kernel.h>
#include <user/name.h>
#include <user/train.h>
#include <user/io.h>
#include <test.h>

extern Track singleton_track;
static int io_tid;
static int train_tid;

static void test_reverse_simple() {
    TrackNode *node = NULL;
    switch (singleton_track.name) {
        case TRAIN_TRACK_A:
            node = track_find_node_by_name(&singleton_track, "B15");
            break;
        case TRAIN_TRACK_B:
            node = track_find_node_by_name(&singleton_track, "B16");
            break;
    }
    TrainMove(train_tid, TRAIN_ID, TRAIN_SPEED, node, 0);
    Printf(io_tid, COM2, "Check expected destination %s\n\r", node->name);
}

static void test_reverse_onspot() {
    TrackNode *node = NULL;
    switch (singleton_track.name) {
        case TRAIN_TRACK_A:
            node = track_find_node_by_name(&singleton_track, NODE_TRACK_A)->reverse;
            break;
        case TRAIN_TRACK_B:
            node = track_find_node_by_name(&singleton_track, NODE_TRACK_B)->reverse;
            break;
    }
    TrainMove(train_tid, TRAIN_ID, TRAIN_SPEED, node, 0);
    Printf(io_tid, COM2, "Check expected position %s\n\r", node->name);
}

static struct {
    const char *name;
    void (*func)();
} reversetable[] = {
    { "Reverse simple",    test_reverse_simple },
    { "Reverse onspot",    test_reverse_onspot },
    { NULL,                NULL                },
};

int test_reverse(int argc, char **argv) {
    (void)argc;
    (void)argv;
    io_tid = WhoIs(SERVER_NAME_IO);
    train_tid = WhoIs(SERVER_NAME_TRAIN);

    basic_setup(io_tid, train_tid);
    test_reverse_simple();
    /*test_reverse_onspot();*/
    return 0;
}
