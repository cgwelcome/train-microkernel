#include <hardware/timer.h>
#include <train/manager.h>
#include <train/controller.h>
#include <train/train.h>
#include <user/ui.h>
#include <user/io.h>
#include <utils/assert.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TERM_ENABLE_SCROLL "\033[%u;%ur"
#define TERM_ERASE_LINE    "\033[K"
#define TERM_MOVE_CURSOR   "\033[%u;%uH"
#define TERM_SAVE_CURSOR   "\033[s"
#define TERM_UNSAVE_CURSOR "\033[u"

#define TERM_CURSOR_COLOR_RED  "\033[31m"
#define TERM_CURSOR_COLOR_NONE "\033[0m"

void PrintBasicInterface(int io_tid) {
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "CPU Idle Rate:"                               , LINE_IDLE              , 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "Time: "                                       , LINE_TIME              , 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "Switches: "                                   , LINE_SWITCH_TITLE      , 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "    1:C    2:C    3:C    4:C    5:C    6:C"   , LINE_SWITCH_START + 0  , 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "    7:C    8:C    9:C   10:C   11:C   12:C"   , LINE_SWITCH_START + 1  , 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "   13:C   14:C   15:C   16:C   17:C   18:C"   , LINE_SWITCH_START + 2  , 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "  153:C  154:C  155:C  156:C              "   , LINE_SWITCH_START + 3  , 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "Locations: "                                  , LINE_LOCATION_TITLE    , 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "  Train  1: "                                 , LINE_LOCATION_START + 0, 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "  Train 24: "                                 , LINE_LOCATION_START + 1, 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "  Train 58: "                                 , LINE_LOCATION_START + 2, 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "  Train 74: "                                 , LINE_LOCATION_START + 3, 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "  Train 78: "                                 , LINE_LOCATION_START + 4, 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "  Train 79: "                                 , LINE_LOCATION_START + 5, 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "Sensors: ID, Train ID, Error"                 , LINE_SENSOR_TITLE      , 1);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR "> █"                                          , LINE_TERMINAL          , 1);
    Printf(io_tid, COM2, TERM_ENABLE_SCROLL                                              , LINE_LOG_START         , LINE_LOG_END);
    Printf(io_tid, COM2, TERM_MOVE_CURSOR                                                , LINE_LOG_START         , 1);
}

void PrintIdle(int io_tid, int usage) {
    Printf(io_tid, COM2,
            TERM_SAVE_CURSOR TERM_MOVE_CURSOR TERM_ERASE_LINE "%u.%02u%%" TERM_UNSAVE_CURSOR,
            LINE_IDLE, 16,
            usage / 100, usage % 100);
}

void PrintTime(int io_tid) {
    uint32_t now = (uint32_t) timer_read(TIMER3);
    Printf(io_tid, COM2,
            TERM_SAVE_CURSOR TERM_MOVE_CURSOR TERM_ERASE_LINE "%04u:%02u.%u" TERM_UNSAVE_CURSOR,
            LINE_TIME, 7,
            now / 60000     /* minute */,
            now / 1000 % 60 /* second */,
            now / 100  % 10 /* tenth of second */);
}

void PrintSwitch(int io_tid, unsigned int code, uint8_t direction) {
    assert((code > 0 && code < 19) || (code > 0x98 && code < 0x9D));
    assert(direction == DIR_CURVED || direction == DIR_STRAIGHT);
    unsigned int row = 0, col = 0;
    char status = (direction == DIR_CURVED ? 'C' : 'S');
    if (code > 0 && code < 19) {
        row = (code - 1) / 6;
        col = (code - 1) % 6;
    }
    if (code > 0x98 && code < 0x9D) {
        row = 3;
        col = code - 0x99;
    }
    Printf(io_tid, COM2,
            TERM_SAVE_CURSOR TERM_MOVE_CURSOR "%c" TERM_UNSAVE_CURSOR,
            LINE_SWITCH_START + row, 7 + col * 7,
            status);
}

void PrintLocation(int io_tid, Train *train) {
    TrackPosition *pos  = &train->position;
    TrackPosition *stop = &train->final_position;
    TrackPosition *reverse = &train->reverse_position;
    if (pos->edge != NULL) {
        uint32_t row = train_id_to_index(train->id);
        Printf(io_tid, COM2,
                TERM_SAVE_CURSOR TERM_MOVE_CURSOR TERM_ERASE_LINE "%u %s %u %u %s %u %s %u %s %u %d %d" TERM_UNSAVE_CURSOR,
                LINE_LOCATION_START + row, 13,
                train->miss_count, train->missed_sensor == NULL ? "N" : train->missed_sensor->name,
                train->speed, train->original_speed,
                pos->edge     == NULL ? "N" : pos->edge->src->name,  pos->offset,
                stop->edge    == NULL ? "N" : stop->edge->src->name, stop->offset,
                reverse->edge == NULL ? "N" : reverse->edge->src->name, reverse->offset,
                train->state,
                stop->edge == NULL ? 0 : train_close_to(train, *stop, 200));
    }
}

void PrintSensor(int io_tid, TrainSensor *sensor, uint32_t time) {
    Printf(io_tid, COM2,
            TERM_SAVE_CURSOR TERM_ENABLE_SCROLL TERM_MOVE_CURSOR TERM_ERASE_LINE "  Hit %c%u at %u\r\n" TERM_ENABLE_SCROLL TERM_UNSAVE_CURSOR,
            LINE_SENSOR_START, LINE_SENSOR_END,
            LINE_SENSOR_END, 1,
            sensor->module, sensor->id, time,
            LINE_LOG_START, LINE_LOG_END);
}

void PrintTerminal(int io_tid, const char *buffer) {
    Printf(io_tid, COM2,
            TERM_SAVE_CURSOR TERM_MOVE_CURSOR TERM_ERASE_LINE "%s█" TERM_UNSAVE_CURSOR,
            LINE_TERMINAL, 3,
            buffer);
}

void PrintWarning(int io_tid, char *fmt, ...) {
    va_list va;

    va_start(va, fmt);
    char buffer[256]; vsnprintf(buffer, 256, fmt, va);
    Printf(io_tid, COM2,
            TERM_SAVE_CURSOR TERM_MOVE_CURSOR TERM_ERASE_LINE TERM_CURSOR_COLOR_RED "%s" TERM_CURSOR_COLOR_NONE TERM_UNSAVE_CURSOR,
            LINE_WARNING, 1,
            buffer);
    va_end(va);
}

void PrintPath(int io_tid, TrackPath *path) {
    if (path->dist == 0) {
        Printf(io_tid, COM2, "Path not found\n\r");
        return;
    }
    Printf(io_tid, COM2, "%s -> ", path->list.edges[0]->src->name);
    for (uint32_t i = 0; i < path->list.size-1; i++) {
        Printf(io_tid, COM2, "%s -> ", path->list.edges[i]->dest->name);
    }
    Printf(io_tid, COM2, "%s\n\r", path->list.edges[path->list.size-1]->dest->name);
    Printf(io_tid, COM2, "%d mm\n\r", path->dist);
}

