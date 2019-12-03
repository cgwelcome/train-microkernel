#include <executable.h>
#include <kernel.h>
#include <user/name.h>
#include <user/train.h>
#include <user/io.h>
#include <user/clock.h>

#include <ctype.h>

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

static void exec_collision_reset() {
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

static void exec_collision_rest_headon() {
    exec_collision_reset();
    TrainReverse(train_tid, train2_id);
    TrainMove(train_tid, train1_id, 10, train2_node, 0);
}

static void exec_collision_rest_bump() {
    exec_collision_reset();
    TrainMove(train_tid, train1_id, 10, train2_node, 0);
}

// Track B
static void exec_collision_rest_switch() {
    TrackNode *node1 = track_find_node_by_name(&singleton_track, "E7");
    TrackNode *node2 = track_find_node_by_name(&singleton_track, "D3");
    basic_train_setup(train1_id, node1);
    basic_train_setup(train2_id, node2);
    Printf(io_tid, COM2, "Wait a GO signal [Enter]!\n\r");
    Getc(io_tid, COM2, NULL);
    Printf(io_tid, COM2, "GO!\n\r");
    TrainSpeed(train_tid, train1_id, 10);
    TrainSpeed(train_tid, train2_id, 10);
}

static void exec_collision_congestion() {
    exec_collision_reset();
    TrainSwitch(train_tid, 8, DIR_STRAIGHT);
    TrainSpeed(train_tid, train1_id, 12);
    TrainSpeed(train_tid, train2_id, 10);
}

// Track B
static void exec_collision_reverse_ahead() {
    exec_collision_reset();
    TrainSwitch(train_tid, 8, DIR_STRAIGHT);
    TrainSpeed(train_tid, train1_id, 10);
    TrainSpeed(train_tid, train2_id, 10);
    char c = 0;
    while (c != 'q') {
        Printf(io_tid, COM2, "Reversing\n\r");
        Printf(io_tid, COM2, "Waiting a reverse signal [Enter]!\n\r");
        Getc(io_tid, COM2, &c);
        TrainReverse(train_tid, train2_id);
    }
}


static Executable collision_suite[] = {
    { "Head-on Rest Collision",   exec_collision_rest_headon   },
    { "Bump Rest Collision",      exec_collision_rest_bump     },
    { "Switch Rest Collision",    exec_collision_rest_switch   },
    { "Reverse Ahead Collision",  exec_collision_reverse_ahead },
    { "Congestion Collision",     exec_collision_congestion    },
    { "Reset Collision",          exec_collision_reset         },
    { NULL,                       NULL                         },
};

int exec_collision(int argc, char **argv) {
    (void)argc;
    (void)argv;
    io_tid = WhoIs(SERVER_NAME_IO);
    train_tid = WhoIs(SERVER_NAME_TRAIN);
    uint32_t size = sizeof(collision_suite)/sizeof(collision_suite[0]);
    basic_track_setup();
    basic_menu(collision_suite, size);
    return 0;
}
