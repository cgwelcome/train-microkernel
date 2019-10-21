#include <server/clock.h>
#include <server/io.h>
#include <server/shell.h>
#include <server/trainset.h>
#include <user/io.h>
#include <user/clock.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/trainset.h>
#include <utils/queue.h>
#include <utils/bwio.h>
#include <user/trainset.h>

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

static void shell_print_interface(int iotid) {
    Printf(iotid, COM2, "\033[%u;%uHTIME: "                          , LINE_TIME            , 1);
    Printf(iotid, COM2, "\033[%u;%uHSWITCHES: "                      , LINE_SWITCH_TITLE    , 1);
    Printf(iotid, COM2, "\033[%u;%uH  01:C 02:C 03:C 04:C 05:C 06:C ", LINE_SWITCH_START + 0, 1);
    Printf(iotid, COM2, "\033[%u;%uH  07:C 08:C 09:C 10:C 11:C 12:C ", LINE_SWITCH_START + 1, 1);
    Printf(iotid, COM2, "\033[%u;%uH  13:C 14:C 15:C 16:C 17:C 18:C ", LINE_SWITCH_START + 2, 1);
    Printf(iotid, COM2, "\033[%u;%uH  99:C 9A:C 9B:C 9C:C "          , LINE_SWITCH_START + 3, 1);
    Printf(iotid, COM2, "\033[%u;%uHSENSORS: "                       , LINE_SENSOR_TITLE    , 1);
    Printf(iotid, COM2, "\033[%u;%uH> █"                             , LINE_TERMINAL        , 1);
}

static void shell_print_terminal(char *cmd_buffer, unsigned int cmd_len, int iotid) {
    cmd_buffer[cmd_len] = '\0';
    Printf(iotid, COM2, "\033[%u;%uH\033[K%s█",
        LINE_TERMINAL, 4,
        cmd_buffer
    );
}

static void shell_execute_command(char *cmd_buffer, unsigned int cmd_len, int traintid) {
    int arg1_len, arg2_len, code, speed, direction;
    switch (cmd_buffer[0]) {
        case 't':                // set train speed
            arg1_len = find(cmd_buffer + 3, (int) cmd_len - 3, ' ');
            code     = atoi(cmd_buffer + 3, arg1_len);
            arg2_len = (int)cmd_len - 3 - arg1_len - 1;
            speed    = atoi(cmd_buffer + 3 + arg1_len + 1, arg2_len);
            if ((code > 0 && code < 81) && (speed >= 0 && speed <= 14)) {
                Trainset_Speed(traintid, (uint32_t)code, (uint32_t)speed);
            }
            break;
        case 'r':                // reverse the train
            code = atoi(cmd_buffer + 3, (int)cmd_len - 3);
            if (code > 0 && code < 81) {
                Trainset_Reverse(traintid, (uint32_t)code);
            }
            break;
        case 's':                // turn on/off the switch
            arg1_len  = find(cmd_buffer + 3, (int)cmd_len - 3, ' ');
            code      = atoi(cmd_buffer + 3, arg1_len);
            direction = (cmd_buffer + 3)[arg1_len + 1];
            if (direction != 'S' && direction != 'C') break;
            if ((code > 0 && code < 19) || (code > 0x98 && code < 0x9D)) {
                TrainSwitchStatus status;
                switch (direction) {
                    case 'C':
                        status = SWITCHSTATUS_CURVED;
                        break;
                    case 'S':
                        status =  SWITCHSTATUS_STRAIGHT;
                        break;
                    default:
                        return;
                        break;
                }
                Trainset_Switchone(traintid, (uint32_t)code, status);
            }
            break;
    }
}


static void shell_keyboard_task() {
    unsigned int cmd_len = 0;
    char cmd_buffer[CMD_BUFFER_SIZE];

    int iotid = WhoIs(IO_SERVER_NAME);
    int traintid = WhoIs(TRAINSET_SERVER_NAME);
    for (;;) {
        char in = (char) Getc(iotid, COM2);
        switch (in) {
            case '\r':                         // execute command if get "ENTER"
                if (cmd_len != 0) {
                    if (cmd_buffer[0] == 'q') {
                        Trainset_Done(traintid);
                        ShutdownIOServer();
                        Shutdown();
                    }
                    shell_execute_command(cmd_buffer, cmd_len, traintid);
                    cmd_len = 0;
                    shell_print_terminal(cmd_buffer, cmd_len, iotid);
                }
                break;
            case '\b':                         // delete character if get "BACKSPACE"
                if (cmd_len != 0) {
                    cmd_len -= 1;
                    shell_print_terminal(cmd_buffer, cmd_len, iotid);
                }
                break;
            default:                           // otherwise just store the character
                if (cmd_len < CMD_BUFFER_SIZE) {
                    cmd_buffer[cmd_len] = in;
                    cmd_len += 1;
                    shell_print_terminal(cmd_buffer, cmd_len, iotid);
                }
                break;
        }

    }
    Exit();
}

static void shell_clock_init(ShellClock *shellclock) {
    shellclock->minute = 0;
    shellclock->second = 0;
    shellclock->decisecond = 0;
}

static void shell_clock_update(ShellClock *shellclock) {
    if (shellclock->decisecond < 9) {
        shellclock->decisecond++;
        return;
    }
    shellclock->decisecond = 0;
    if (shellclock->second < 59) {
        shellclock->second++;
        return;
    }
    shellclock->second = 0;
    shellclock->minute++;
}

static void shell_clock_display(int iotid, ShellClock *shellclock) {
    Printf(iotid, COM2, "\033[%u;%uH\033[K%04u:%02u:%03u",
        LINE_TIME, 7,
        shellclock->minute,
        shellclock->second,
        shellclock->decisecond
    );
}

static void shell_clock_task() {
    ShellClock shellclock;
    shell_clock_init(&shellclock);
    int clocktid = WhoIs(CLOCK_SERVER_NAME);
    int iotid = WhoIs(IO_SERVER_NAME);
    for (;;) {
        Delay(clocktid, CLOCK_PRECISION/10);
        shell_clock_update(&shellclock);
        shell_clock_display(iotid, &shellclock);
    }
    Exit();
}

static void shell_sensor_task() {
    ActiveTrainSensorList active_list; active_list.size = 0;
    ActiveTrainSensorList output_list; output_list.size = 0;

    int clocktid = WhoIs(CLOCK_SERVER_NAME);
    int iotid = WhoIs(IO_SERVER_NAME);
    int traintid = WhoIs(TRAINSET_SERVER_NAME);

    for (;;) {
        Delay(clocktid, SENSOR_READ_INTERVAL/10);
        active_list = Trainset_Sensor_Readall(traintid);
        for (uint32_t i = 0; i < active_list.size; i++) {
            if (output_list.size < SENSOR_LIST_SIZE) {
                output_list.size += 1;
            }
            for (int t = SENSOR_LIST_SIZE - 1; t > 0; t--) {
                output_list.sensors[t] = output_list.sensors[t - 1];
            }
            output_list.sensors[0] = active_list.sensors[i];
        }
        for (uint32_t i = 0; i < output_list.size; i++) {
            Printf(iotid, COM2, "\033[%d;%dH\033[K%c%u",
                LINE_SENSOR_START + i, 3,
                output_list.sensors[i].module,
                output_list.sensors[i].id
            );
        };
    }
    Exit();
}

void shell_server_root_task() {
    int iotid = WhoIs(IO_SERVER_NAME);
    // Clear the screen
    Printf(iotid, COM2, "\033[2J");
    // Hide the cursor
    Printf(iotid, COM2, "\033[?25l");
    // Initialize the interface
    shell_print_interface(iotid);

    Create(SHELL_PRIORITY, &shell_keyboard_task);
    Create(SHELL_PRIORITY, &shell_clock_task);
    Create(SHELL_PRIORITY, &shell_sensor_task);
    Exit();
}

int CreateShellServer(uint32_t priority) {
    return Create(priority, &shell_server_root_task);
}
