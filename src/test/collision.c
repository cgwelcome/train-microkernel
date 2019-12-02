#include <ctype.h>
#include <kernel.h>
#include <user/name.h>
#include <user/train.h>
#include <user/io.h>
#include <user/clock.h>
#include <test.h>

#define TRAIN2_ID         1
#define NODE2_TRACK_A  "E11"
#define NODE2_TRACK_B   "E7"


extern Track singleton_track;
extern int io_tid;
extern int train_tid;

extern TrackNode *train1_node;
extern uint32_t train1_id;

static TrackNode *train2_node;
static uint32_t train2_id;

static void test_collision_reset() {
    train2_id = TRAIN2_ID;
    basic_setup();
    switch (singleton_track.name) {
        case TRAIN_TRACK_A:
            train2_node = track_find_node_by_name(&singleton_track, NODE2_TRACK_A);
            break;
        case TRAIN_TRACK_B:
            train2_node = track_find_node_by_name(&singleton_track, NODE2_TRACK_B);
            break;
    }
    basic_train_setup(train2_id, train2_node);
}

static void test_collision_rest_headon() {
    TrainReverse(train_tid, train2_id);
    TrainMove(train_tid, train1_id, 10, train2_node, 0);
}

static void test_collision_rest_bump() {
    TrainMove(train_tid, train1_id, 10, train2_node, 0);
}

static void test_collision_rest_switch() {
}

static void test_collision_congestion() {
    TrainSwitch(train_tid, 8, DIR_STRAIGHT);
    TrainSpeed(train_tid, train1_id, 12);
    TrainSpeed(train_tid, train2_id, 10);
}

// Track B
static void test_collision_reverse_ahead() {
    TrainSwitch(train_tid, 8, DIR_STRAIGHT);
    TrainSpeed(train_tid, train1_id, 10);
    TrainSpeed(train_tid, train2_id, 10);
    char c = 0;
    while (c != 'q') {
        Getc(io_tid, COM2, &c);
        TrainReverse(train_tid, train2_id);
    }
}


static TestCase collision_suite[] = {
    { "Head-on Rest Collision",   test_collision_rest_headon   },
    { "Bump Rest Collision",      test_collision_rest_bump     },
    { "Switch Rest Collision",    test_collision_rest_switch   },
    { "Reverse Ahead Collision",  test_collision_reverse_ahead },
    { "Congestion Collision",     test_collision_congestion    },
    { "Reset Collision",          test_collision_reset         },
    { NULL,                       NULL                         },
};

int test_collision(int argc, char **argv) {
    (void)argc;
    (void)argv;
    io_tid = WhoIs(SERVER_NAME_IO);
    train_tid = WhoIs(SERVER_NAME_TRAIN);
    uint32_t size = sizeof(collision_suite)/sizeof(collision_suite[0]);
    test_collision_reset();
    basic_menu(collision_suite, size);
    return 0;
}
