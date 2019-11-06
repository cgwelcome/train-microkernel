#include <kernel.h>
#include <server/io.h>
#include <server/ui.h>
#include <user/io.h>
#include <user/clock.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/train.h>
#include <user/ui.h>
#include <utils/queue.h>
#include <utils/bwio.h>

static int find(char *str, int len, char ch) {
    int i = 0;
    for (i = 0; i < len; i++) {
        if (str[i] == ch) return i;
    }
    return -1;
}

static int atoi(char *str, int len) {
    int result = 0;
    int i = 0;
    for (i = 0; i < len; i++) {
        if (str[i] < '0' || str[i] > '9') return result;
        result = result * 10 + (str[i] - '0');
    }
    return result;
}

static void ui_execute_command(int io_tid, int traintid, char *cmd_buffer, unsigned int cmd_len) {
    int arg1_len, arg2_len, code, speed, direction;
    switch (cmd_buffer[0]) {
        case 't':                // set train speed
            arg1_len = find(cmd_buffer + 3, (int) cmd_len - 3, ' ');
            code     = atoi(cmd_buffer + 3, arg1_len);
            arg2_len = (int)cmd_len - 3 - arg1_len - 1;
            speed    = atoi(cmd_buffer + 3 + arg1_len + 1, arg2_len);
            if ((code > 0 && code < 81) && (speed >= 0 && speed <= 14)) {
                TrainSetSpeed(traintid, (uint32_t)code, (uint32_t)speed);
            }
            break;
        case 'r':                // reverse the train
            code = atoi(cmd_buffer + 3, (int)cmd_len - 3);
            if (code > 0 && code < 81) {
                TrainReverse(traintid, (uint32_t)code);
            }
            break;
        case 's':                // turn on/off the switch
            arg1_len  = find(cmd_buffer + 3, (int)cmd_len - 3, ' ');
            code      = atoi(cmd_buffer + 3, arg1_len);
            direction = (cmd_buffer + 3)[arg1_len + 1];
            if ((code > 0 && code < 19) || (code > 0x98 && code < 0x9D)) {
                TrainSwitchStatus status;
                switch (direction) {
                    case 'C':
                        status = TRAIN_SWITCH_CURVED;
                        break;
                    case 'S':
                        status =  TRAIN_SWITCH_STRAIGHT;
                        break;
                    default:
                        return;
                }
                TrainSwitchOne(traintid, (uint32_t)code, status);
                PrintSwitch(io_tid, (unsigned int) code, (char) direction);
            }
            break;
    }
}

static void ui_clock_task() {
    int clock_tid = WhoIs(SERVER_NAME_CLOCK);
    int io_tid = WhoIs(SERVER_NAME_IO);
    for (;;) {
        Delay(clock_tid, CLOCK_PRECISION/10);
        PrintTime(io_tid);
    }
    Exit();
}

static void ui_keyboard_task() {
    unsigned int cmd_len = 0;
    char cmd_buffer[CMD_BUFFER_SIZE];

    int io_tid = WhoIs(SERVER_NAME_IO);
    int train_tid = WhoIs(SERVER_NAME_TMS);
    for (;;) {
        char in = (char) Getc(io_tid, COM2);
        switch (in) {
            case '\r':                         // execute command if get "ENTER"
                if (cmd_len != 0) {
                    if (cmd_buffer[0] == 'q') {
                        Printf(io_tid, COM2, "\033[%u;%uH", LINE_DEBUG, 1);
                        TrainExit(train_tid);
                        ShutdownIOServer();
                        Shutdown();
                    }
                    ui_execute_command(io_tid, train_tid, cmd_buffer, cmd_len);
                    cmd_len = 0;
                    PrintTerminal(io_tid, cmd_buffer, cmd_len);
                }
                break;
            case '\b':                         // delete character if get "BACKSPACE"
                if (cmd_len != 0) {
                    cmd_len -= 1;
                    PrintTerminal(io_tid, cmd_buffer, cmd_len);
                }
                break;
            default:                           // otherwise just store the character
                if (cmd_len < CMD_BUFFER_SIZE) {
                    cmd_buffer[cmd_len] = in;
                    cmd_len += 1;
                    PrintTerminal(io_tid, cmd_buffer, cmd_len);
                }
                break;
        }
    }
    Exit();
}

void ui_server_root_task() {
    int io_tid = WhoIs(SERVER_NAME_IO);
    // Clear the screen
    Printf(io_tid, COM2, "\033[2J");
    // Hide the cursor
    Printf(io_tid, COM2, "\033[?25l");
    // Initialize the interface
    PrintBasicInterface(io_tid);

    Create(PRIORITY_SERVER_UI, &ui_clock_task);
    Create(PRIORITY_SERVER_UI, &ui_keyboard_task);
    Exit();
}

void CreateUIServer() {
    Create(PRIORITY_SERVER_UI, &ui_server_root_task);
}
