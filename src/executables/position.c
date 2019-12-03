#include <kernel.h>
#include <train/track.h>
#include <user/io.h>
#include <user/name.h>

#include <stdlib.h>

// NOTE: these structures must be read only in this file!
extern Track singleton_track;

void print_position(int io_tid, char *prefix, TrackPosition position) {
    Printf(io_tid, COM2, "%s %s-%s %u\r\n", prefix, position.edge->src->name, position.edge->dest->name, position.offset);
}

int exec_position(int argc, char **argv) {
    int io_tid = WhoIs(SERVER_NAME_IO);
    track_init(&singleton_track, TRAIN_TRACK_A);

    if (argc != 5) {
        Printf(io_tid, COM2, "Usage: exec position [this train position] [other train position]\r\n");
        return 1;
    }
    TrackPosition this_position = {
        .edge   = node_select_next_edge(track_find_node_by_name(&singleton_track, argv[1])),
        .offset = (uint32_t)atoi(argv[2])
    };
    print_position(io_tid, "this ->", this_position);
    TrackPosition other_position = {
        .edge   = node_select_next_edge(track_find_node_by_name(&singleton_track, argv[3])),
        .offset = (uint32_t)atoi(argv[4]),
    };
    print_position(io_tid, "other ->", other_position);

    TrackPosition detection_end = position_move(this_position, 1000);
    print_position(io_tid, "detection end ->", detection_end);
    TrackPosition other_reversed = position_reverse(other_position);
    print_position(io_tid, "other reversed ->", other_reversed);

    uint32_t result = position_dist(this_position, other_reversed, 1000);
    Printf(io_tid, COM2, "Final Result: %u\r\n", result);

    return 0;
}
