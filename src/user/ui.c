#include <hardware/timer.h>
#include <train/train.h>
#include <train/trainset.h>
#include <user/ui.h>
#include <user/io.h>
#include <utils/assert.h>

void PrintBasicInterface(int io_tid) {
    Printf(io_tid, COM2, "\033[%u;%uHCPU Idle Rate:"                               , LINE_IDLE              , 1);
    Printf(io_tid, COM2, "\033[%u;%uHTime: "                                       , LINE_TIME              , 1);
    Printf(io_tid, COM2, "\033[%u;%uHSwitches: "                                   , LINE_SWITCH_TITLE      , 1);
    Printf(io_tid, COM2, "\033[%u;%uH    1:C    2:C    3:C    4:C    5:C    6:C"   , LINE_SWITCH_START + 0  , 1);
    Printf(io_tid, COM2, "\033[%u;%uH    7:C    8:C    9:C   10:C   11:C   12:C"   , LINE_SWITCH_START + 1  , 1);
    Printf(io_tid, COM2, "\033[%u;%uH   13:C   14:C   15:C   16:C   17:C   18:C"   , LINE_SWITCH_START + 2  , 1);
    Printf(io_tid, COM2, "\033[%u;%uH  153:C  154:C  155:C  156:C              "   , LINE_SWITCH_START + 3  , 1);
    Printf(io_tid, COM2, "\033[%u;%uHLocations: "                                  , LINE_LOCATION_TITLE    , 1);
    Printf(io_tid, COM2, "\033[%u;%uH  Train  1: "                                 , LINE_LOCATION_START + 0, 1);
    Printf(io_tid, COM2, "\033[%u;%uH  Train 24: "                                 , LINE_LOCATION_START + 1, 1);
    Printf(io_tid, COM2, "\033[%u;%uH  Train 58: "                                 , LINE_LOCATION_START + 2, 1);
    Printf(io_tid, COM2, "\033[%u;%uH  Train 74: "                                 , LINE_LOCATION_START + 3, 1);
    Printf(io_tid, COM2, "\033[%u;%uH  Train 78: "                                 , LINE_LOCATION_START + 4, 1);
    Printf(io_tid, COM2, "\033[%u;%uH  Train 79: "                                 , LINE_LOCATION_START + 5, 1);
    Printf(io_tid, COM2, "\033[%u;%uHSensors: ID, Train ID"                        , LINE_SENSOR_TITLE      , 1);
    Printf(io_tid, COM2, "\033[%u;%uH> █"                                          , LINE_TERMINAL          , 1);
}

void PrintIdle(int io_tid, int usage) {
    Printf(io_tid, COM2, "\033[s\033[%u;%uH\033[K%u.%u%%\033[u", LINE_IDLE, 16, usage / 100, usage % 100);
}

void PrintTime(int io_tid) {
    uint32_t now = (uint32_t) timer_read(TIMER3);
    Printf(io_tid, COM2, "\033[s\033[%u;%uH\033[K%04u:%02u.%u\033[u",
            LINE_TIME, 7,
            now / 60000,        // minute
            now / 1000 % 60,    // second
            now / 100  % 10     // tenth of second
          );
}

void PrintSwitch(int io_tid, unsigned int code, char direction) {
    unsigned int row = 0, col = 0;
    assert((code > 0 && code < 19) || (code > 0x98 && code < 0x9D));
    if (code > 0 && code < 19) {
        row = (code - 1) / 6;
        col = (code - 1) % 6;
    }
    if (code > 0x98 && code < 0x9D) {
        row = 3;
        col = code - 0x99;
    }
    char status = (direction == DIR_CURVED ? 'C' : 'S');
    Printf(io_tid, COM2, "\033[s\033[%u;%uH%c\033[u", LINE_SWITCH_START + row, 7 + col * 7, status);
}

void PrintVelocity(int io_tid, uint32_t train_id, uint32_t time, uint32_t velocity) {
    uint32_t row = train_id_to_index(train_id);
    Printf(io_tid, COM2, "\033[s\033[%u;%uH\033[K%u %u\033[u", LINE_LOCATION_START + row, 13, time, velocity);
}

void PrintTimeDifference(int io_tid, uint32_t train_id, uint64_t expected_time) {
    uint32_t row = train_id_to_index(train_id);
    uint64_t now = timer_read(TIMER3);
    Printf(io_tid, COM2, "\033[s\033[%u;%uH\033[K%d\033[u" , LINE_LOCATION_START + row, 13, ((int) now - (int) expected_time) / 100);
}

void PrintTerminal(int io_tid, const char *buffer) {
    Printf(io_tid, COM2, "\033[%u;%uH\033[K%s", LINE_TERMINAL, 3, buffer);
}

void PrintSensors(int io_tid, ActiveTrainSensorList *list) {
    for (uint32_t i = 0; i < list->size; i++) {
        Printf(io_tid, COM2, "\033[s\033[%d;%dH\033[K%c%u\033[u",
            LINE_SENSOR_START + i, 3,
            list->sensors[i].module,
            list->sensors[i].id
        );
        Train *train = list->sensors[i].train;
        if (train == NULL) {
            Printf(io_tid, COM2, "\033[s\033[%d;%dH%s\033[u",
                LINE_SENSOR_START + i, 8,
                "?"
            );
        } else {
            Printf(io_tid, COM2, "\033[s\033[%d;%dH%d\033[u",
                LINE_SENSOR_START + i, 8,
                train->id
            );
        }
    };

}
