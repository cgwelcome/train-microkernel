#include <kernel.h>
#include <train/track.h>
#include <user/io.h>
#include <user/name.h>

#include <stdlib.h>

// NOTE: these structures must be read only in this file!
extern Track singleton_track;

void print_position(int io_tid, char *prefix, TrackPosition position) {
    Printf(io_tid, COM2, "%s %s %u\r\n", prefix, position.node->name, position.offset);
}

int exec_position(int argc, char **argv) {
    int io_tid = WhoIs(SERVER_NAME_IO);
    track_init(&singleton_track, TRAIN_TRACK_A);

    if (argc != 5) {
        Printf(io_tid, COM2, "Usage: exec position [this train position] [other train position]\r\n");
        return 1;
    }
    TrackPosition this_position = {
        .node   = track_find_node_by_name(&singleton_track, argv[1]),
        .offset = (uint32_t)atoi(argv[2])
    };
    print_position(io_tid, "this ->", this_position);
    TrackPosition other_position = {
        .node   = track_find_node_by_name(&singleton_track, argv[3]),
        .offset = (uint32_t)atoi(argv[4]),
    };
    print_position(io_tid, "other ->", other_position);

    TrackPosition detect_range_start = this_position;
    print_position(io_tid, "detect range start ->", detect_range_start);
    TrackPosition detect_range_end   = position_move(detect_range_start, 1000);
    print_position(io_tid, "detect range end   ->", detect_range_end);
    TrackPosition other_reversed = position_reverse(other_position);
    print_position(io_tid, "other reversed ->", other_reversed);

    int result = position_in_range(other_reversed, detect_range_start, detect_range_end);
    Printf(io_tid, COM2, "Final Result: %d\r\n", result);

    return 0;
}
