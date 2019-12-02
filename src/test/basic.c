#include <ctype.h>
#include <kernel.h>
#include <test.h>
#include <user/name.h>
#include <user/train.h>
#include <user/io.h>

extern Track singleton_track;

int test_setup(int argc, char **argv) {
    (void)argc;
    (void)argv;
    int io_tid = WhoIs(SERVER_NAME_IO);
    int train_tid = WhoIs(SERVER_NAME_TRAIN);
    basic_setup(io_tid, train_tid);
    return 0;
}

void basic_setup(int io_tid, int train_tid) {
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
    TrainInitTrain(train_tid, TRAIN_ID, node);
    char confirm = 'n';
    while (confirm != 'y') {
        Printf(io_tid, COM2, "Initialize train %u on %s [y] \n\r", TRAIN_ID, node->name);
        Getc(io_tid, COM2, &confirm);
    }
}
