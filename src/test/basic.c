#include <ctype.h>
#include <kernel.h>
#include <test.h>
#include <user/name.h>
#include <user/train.h>
#include <user/io.h>

#define TRAIN1_ID       24
#define NODE1_TRACK_A  "A8"
#define NODE1_TRACK_B  "A13"
#define TRAIN_SPEED    10

extern Track singleton_track;
int io_tid;
int train_tid;
TrackNode *train1_node;
uint32_t train1_id;
uint32_t train_speed;

void basic_track_setup() {
    Printf(io_tid, COM2, "Input track name [a/b]\n\r");
    char track_name;
    Getc(io_tid, COM2, &track_name);
    track_name = (char)toupper(track_name);
    switch (track_name) {
        case 'A':
            TrainInitTrack(train_tid, TRAIN_TRACK_A);
            train1_node = track_find_node_by_name(&singleton_track, NODE1_TRACK_A);
            break;
        case 'B':
            TrainInitTrack(train_tid, TRAIN_TRACK_B);
            train1_node = track_find_node_by_name(&singleton_track, NODE1_TRACK_B);
            break;
    }
}

void basic_train_setup(uint32_t train_id, TrackNode *node) {
    Printf(io_tid, COM2, "Check train1 %u is not controlled by Marklin device\n\r", train_id);
    TrainInitTrain(train_tid, train_id, node);
    char confirm = 'n';
    while (confirm != 'y') {
        Printf(io_tid, COM2, "Initialize train %u on %s [y] \n\r", train_id, node->name);
        Getc(io_tid, COM2, &confirm);
    }
}

void basic_setup() {
    if (!singleton_track.inited) {
        basic_track_setup();
    }
    train1_id = TRAIN1_ID;
    train_speed = TRAIN_SPEED;
    basic_train_setup(train1_id, train1_node);
}

static void basic_print_menu(TestCase *suite) {
    for (uint32_t i = 0; suite[i].name; i++) {
        Printf(io_tid, COM2, "[%u] : %s\n\r", i, suite[i].name);
    }
    Printf(io_tid, COM2, "[b] : boomerang\n\r");
    Printf(io_tid, COM2, "[r] : reset\n\r");
    Printf(io_tid, COM2, "[q] : quit\n\r");
}


void basic_menu(TestCase *suite, uint32_t size) {
    char option = 0;
    for (;;) {
        basic_print_menu(suite);
        Printf(io_tid, COM2, "Please enter your choice:\n\r");
        Getc(io_tid, COM2, &option);
        if ('0' <= option && option <= '0'+size-1) {
            uint32_t i = (uint32_t)option-'0';
            Printf(io_tid, COM2, "Excuting test %u: %s\n\r", i, suite[i].name);
            suite[i].func();
        } else if (option == 'b') {
            TrainMove(train_tid, train1_id, train_speed, train1_node, 0);
        } else if (option == 'r') {
            basic_setup();
        } else if (option == 'q') {
            break;
        } else {
            Printf(io_tid, COM2, "Invalid option\n\r");
        }
    }
    Printf(io_tid, COM2, "Quitting...\n\r");
}

int test_setup(int argc, char **argv) {
    (void)argc;
    (void)argv;
    io_tid = WhoIs(SERVER_NAME_IO);
    train_tid = WhoIs(SERVER_NAME_TRAIN);
    basic_setup();
    return 0;
}

