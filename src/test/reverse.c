#include <ctype.h>
#include <kernel.h>
#include <user/name.h>
#include <user/train.h>
#include <user/io.h>

#define TRAIN_SPEED    10
#define TRAIN_ID       58
#define NODE_TRACK_A  "A8"
#define NODE_TRACK_B  "A13"

extern Track singleton_track;
static int io_tid;
static int train_tid;

static void test_reverse_simple() {
    TrackNode *node = NULL;
    switch (singleton_track.name) {
        case TRAIN_TRACK_A:
            TrainInitTrack(train_tid, TRAIN_TRACK_A);
            node = track_find_node_by_name(&singleton_track, "B15");
            break;
        case TRAIN_TRACK_B:
            TrainInitTrack(train_tid, TRAIN_TRACK_B);
            node = track_find_node_by_name(&singleton_track, "A15");
            break;
    }
    TrainMove(train_tid, TRAIN_ID, TRAIN_SPEED, node, 0);
    Printf(io_tid, COM2, "Check expected destination %s\n\r", node->name);
}

static void test_reverse_onspot() {
    TrackNode *node = NULL;
    switch (singleton_track.name) {
        case TRAIN_TRACK_A:
            TrainInitTrack(train_tid, TRAIN_TRACK_A);
            node = track_find_node_by_name(&singleton_track, NODE_TRACK_A)->reverse;
            break;
        case TRAIN_TRACK_B:
            TrainInitTrack(train_tid, TRAIN_TRACK_B);
            node = track_find_node_by_name(&singleton_track, NODE_TRACK_B)->reverse;
            break;
    }
    TrainMove(train_tid, TRAIN_ID, TRAIN_SPEED, node, 0);
    Printf(io_tid, COM2, "Check expected position %s\n\r", node->name);
    Printf(io_tid, COM2, "Check train light direction\n\r");
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

    Printf(io_tid, COM2, "Input track name [a/b]\n\r");
    char track_name;
    Getc(io_tid, COM2, &track_name);
    track_name = (char)toupper(track_name);
    TrackNode *node = NULL;
    switch (track_name) {
        case 'A':
            TrainInitTrack(train_tid, TRAIN_TRACK_A);
            node = track_find_node_by_name(&singleton_track, NODE_TRACK_A);
            break;
        case 'B':
            TrainInitTrack(train_tid, TRAIN_TRACK_B);
            node = track_find_node_by_name(&singleton_track, NODE_TRACK_B);
            break;
    }
    Printf(io_tid, COM2, "Check train %u is not controlled by Marklin device\n\r", TRAIN_ID);
    Printf(io_tid, COM2, "Initialize train %u on %s [Enter] \n\r", TRAIN_ID, node->name);
    Getc(io_tid, COM2, NULL);
    TrainInitTrain(train_tid, TRAIN_ID, node);
    test_reverse_simple();
    return 0;
}
