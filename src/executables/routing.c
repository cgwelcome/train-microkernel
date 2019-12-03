#include <executable.h>
#include <kernel.h>
#include <user/name.h>
#include <user/train.h>
#include <user/io.h>

extern Track singleton_track;

extern int io_tid;
extern int train_tid;

static char *trip_pool[] = {
    "B15", "B16", "C9" , "E7" , "C5" , "D13", "D1", "D11",
};
    /*{"E5" , 0, "E2" , 0},*/
    /*{"C13", 0, "E15", 0},*/
    /*{"D11", 0, "E16", 0},*/
    /*{"E5" , 0, "A3" , 0},*/

static void exec_routing_taxi() {
    TrackNode *node = track_find_node_by_name(&singleton_track, "A13");
    basic_train_setup(24, node);
    char option = 0;
    uint32_t cursor = 0;
    uint32_t size = sizeof(trip_pool)/sizeof(trip_pool[0]);
    while (cursor < size) {
        Printf(io_tid, COM2, "Press enter to go next dest: %s\n\r", trip_pool[cursor]);
        Getc(io_tid, COM2, &option);
        if (option == 'q') {
            break;
        }
        TrackNode *node = track_find_node_by_name(&singleton_track, trip_pool[cursor]);
        TrainMove(train_tid, 24, 10, node, 0);
        cursor++;
    }
}

static void exec_routing_change() {
    TrackNode *node = track_find_node_by_name(&singleton_track, "A13");
    basic_train_setup(24, node);
    node = track_find_node_by_name(&singleton_track, "B6");
    basic_train_setup(1, node);
    Getc(io_tid, COM2, NULL);

    node = track_find_node_by_name(&singleton_track, "A3");
    Printf(io_tid, COM2, "Moving to %s\n\r", node->name);
    TrainMove(train_tid, 24, 10, node, 0);
    Getc(io_tid, COM2, NULL);

    node = track_find_node_by_name(&singleton_track, "D4");
    Printf(io_tid, COM2, "Moving to %s\n\r", node->name);
    TrainMove(train_tid, 24, 10, node, 0);
}

static Executable routing_suite[] = {
    { "Routing Taxi",       exec_routing_taxi   },
    { "Re-routing",         exec_routing_change },
    { NULL,                 NULL                },
};


int exec_routing(int argc, char **argv) {
    (void)argc;
    (void)argv;
    uint32_t size = sizeof(routing_suite)/sizeof(routing_suite[0]);
    io_tid = WhoIs(SERVER_NAME_IO);
    train_tid = WhoIs(SERVER_NAME_TRAIN);
    basic_track_setup();
    basic_menu(routing_suite, size);
    Printf(io_tid, COM2, "Exit routing test...\n\r");
    return 0;
}
