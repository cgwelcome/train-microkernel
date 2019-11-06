#include <hardware/timer.h>
#include <user/ui.h>
#include <utils/assert.h>

void PrintBasicInterface(int io_tid) {
    Printf(io_tid, COM2, "\033[%u;%uHTIME: "                          , LINE_TIME            , 1);
    Printf(io_tid, COM2, "\033[%u;%uHSWITCHES: "                      , LINE_SWITCH_TITLE    , 1);
    Printf(io_tid, COM2, "\033[%u;%uH  01:C 02:C 03:C 04:C 05:C 06:C ", LINE_SWITCH_START + 0, 1);
    Printf(io_tid, COM2, "\033[%u;%uH  07:C 08:C 09:C 10:C 11:C 12:C ", LINE_SWITCH_START + 1, 1);
    Printf(io_tid, COM2, "\033[%u;%uH  13:C 14:C 15:C 16:C 17:C 18:C ", LINE_SWITCH_START + 2, 1);
    Printf(io_tid, COM2, "\033[%u;%uH  99:C 9A:C 9B:C 9C:C "          , LINE_SWITCH_START + 3, 1);
    Printf(io_tid, COM2, "\033[%u;%uHSENSORS: "                       , LINE_SENSOR_TITLE    , 1);
    Printf(io_tid, COM2, "\033[%u;%uH> █"                             , LINE_TERMINAL        , 1);
}

void PrintTime(int io_tid) {
    uint32_t now = (uint32_t) timer_read(TIMER3);
    Printf(io_tid, COM2, "\033[%u;%uH\033[K%04u:%02u.%u",
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
    Printf(io_tid, COM2, "\033[%u;%uH%c", LINE_SWITCH_START + row, 6 + col * 5, direction);
}

void PrintSensors(int io_tid, TrainSensor *list, size_t size) {
    for (uint32_t i = 0; i < size; i++) {
        Printf(io_tid, COM2, "\033[%d;%dh\033[k%c%u",
            LINE_SENSOR_START + i, 3,
            list[i].module,
            list[i].id
        );
    };
}

void PrintTerminal(int io_tid, char *cmd_buffer, unsigned int cmd_len) {
    cmd_buffer[cmd_len] = '\0';
    Printf(io_tid, COM2, "\033[%u;%uH\033[K%s█",
        LINE_TERMINAL, 3,
        cmd_buffer
    );
}
