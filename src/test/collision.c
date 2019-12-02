#include <ctype.h>
#include <kernel.h>
#include <user/name.h>
#include <user/train.h>
#include <user/io.h>
#include <test.h>

#define TRAIN2_ID        1
#define NODE2_TRACK_A  "A8"
#define NODE2_TRACK_B  "A13"

extern Track singleton_track;
extern int io_tid;
extern int train_tid;

extern TrackNode *train1_node;
extern uint32_t train1_id;
extern uint32_t train_speed;

static TrackNode *train2_node;
static uint32_t train2_id;

static void test_collision_headon() {
}

static void test_collision_headback() {
}

static void test_collision_switch() {
}

static void test_collision_reset() {
    train2_id = TRAIN2_ID;
    basic_setup();
    switch (singleton_track.name) {
        case TRAIN_TRACK_A:
            TrainInitTrack(train_tid, TRAIN_TRACK_A);
            train2_node = track_find_node_by_name(&singleton_track, NODE2_TRACK_A);
            break;
        case TRAIN_TRACK_B:
            TrainInitTrack(train_tid, TRAIN_TRACK_B);
            train2_node = track_find_node_by_name(&singleton_track, NODE2_TRACK_B);
            break;
    }
    basic_train_setup(train2_id, train2_node);
}

static TestCase collision_suite[] = {
    { "Head-on Collision",   test_collision_headon   },
    { "Head-back Collision", test_collision_headback },
    { "Switch Collision",    test_collision_switch   },
    { "Reset Collision",     test_collision_reset    },
    { NULL,                  NULL                    },
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
