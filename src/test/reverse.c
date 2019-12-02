#include <ctype.h>
#include <kernel.h>
#include <user/name.h>
#include <user/train.h>
#include <user/io.h>
#include <test.h>

extern Track singleton_track;
extern int io_tid;
extern int train_tid;

extern TrackNode *train1_node;
extern uint32_t train1_id;
extern uint32_t train1_speed;

static void test_reverse_onspot() {
    TrackNode *node = train1_node->reverse;
    TrainMove(train_tid, train1_id, train1_speed, node, 0);
    Printf(io_tid, COM2, "Check expected position %s\n\r", node->name);
    Printf(io_tid, COM2, "Check orientation %s\n\r", node->name);
}

static void test_reverse_simple() {
    if (!singleton_track.inited) return;
    TrackNode *node = NULL;
    switch (singleton_track.name) {
        case TRAIN_TRACK_A:
            node = track_find_node_by_name(&singleton_track, "B15");
            break;
        case TRAIN_TRACK_B:
            node = track_find_node_by_name(&singleton_track, "B16");
            break;
    }
    TrainMove(train_tid, train1_id, train1_speed, node, 0);
    Printf(io_tid, COM2, "Check expected destination %s\n\r", node->name);
}

static void test_reverse_side_branch() {
    if (!singleton_track.inited) return;
    TrackNode *node0 = NULL;
    TrackNode *node1 = NULL;
    uint32_t alternation = 0;
    char option = 0;
    switch (singleton_track.name) {
        case TRAIN_TRACK_A:
            node0 = track_find_node_by_name(&singleton_track, "A4");
            node1 = track_find_node_by_name(&singleton_track, "B15");
            break;
        case TRAIN_TRACK_B:
            node0 = track_find_node_by_name(&singleton_track, "A15");
            node1 = track_find_node_by_name(&singleton_track, "A12");
            break;
    }
    while (option != 'q') {
        if (alternation == 0) {
            Printf(io_tid, COM2, "Check expected destination %s\n\r", node0->name);
            TrainMove(train_tid, train1_id, train1_speed, node0, 0);
        }
        else {
            Printf(io_tid, COM2, "Check expected destination %s\n\r", node1->name);
            TrainMove(train_tid, train1_id, train1_speed, node1, 0);
        }
        alternation = alternation ^ 1;
        Getc(io_tid, COM2, &option);
    }
}

static TestCase reverse_suite[] = {
    { "Reverse onspot",      test_reverse_onspot      },
    { "Reverse simple",      test_reverse_simple      },
    { "Reverse sidebranch",  test_reverse_side_branch },
    { NULL,                  NULL                     },
};

int test_reverse(int argc, char **argv) {
    (void)argc;
    (void)argv;
    io_tid = WhoIs(SERVER_NAME_IO);
    train_tid = WhoIs(SERVER_NAME_TRAIN);
    uint32_t size = sizeof(reverse_suite)/sizeof(reverse_suite[0]);
    basic_setup();
    basic_menu(reverse_suite, size);
    return 0;
}
