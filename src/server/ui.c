#include <kernel.h>
#include <server/io.h>
#include <server/ui.h>
#include <user/io.h>
#include <user/clock.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/train.h>
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

static void ui_print_interface(int iotid) {
    Printf(iotid, COM2, "\033[%u;%uHTIME: "                          , LINE_TIME            , 1);
    Printf(iotid, COM2, "\033[%u;%uHSWITCHES: "                      , LINE_SWITCH_TITLE    , 1);
    Printf(iotid, COM2, "\033[%u;%uH  01:C 02:C 03:C 04:C 05:C 06:C ", LINE_SWITCH_START + 0, 1);
    Printf(iotid, COM2, "\033[%u;%uH  07:C 08:C 09:C 10:C 11:C 12:C ", LINE_SWITCH_START + 1, 1);
    Printf(iotid, COM2, "\033[%u;%uH  13:C 14:C 15:C 16:C 17:C 18:C ", LINE_SWITCH_START + 2, 1);
    Printf(iotid, COM2, "\033[%u;%uH  99:C 9A:C 9B:C 9C:C "          , LINE_SWITCH_START + 3, 1);
    Printf(iotid, COM2, "\033[%u;%uHSENSORS: "                       , LINE_SENSOR_TITLE    , 1);
    Printf(iotid, COM2, "\033[%u;%uH> █"                             , LINE_TERMINAL        , 1);
}

static void ui_print_switch(int iotid, unsigned int code, char direction) {
    unsigned int row, col;
    if (code > 0 && code < 19) {
        row = (code - 1) / 6;
        col = (code - 1) % 6;
    }
    if (code > 0x98 && code < 0x9D) {
        row = 3;
        col = code - 0x99;
    }
    Printf(iotid, COM2, "\033[%u;%uH%c", LINE_SWITCH_START + row, 6 + col * 5, direction);
}

static void ui_print_terminal(int iotid, char *cmd_buffer, unsigned int cmd_len) {
    cmd_buffer[cmd_len] = '\0';
    Printf(iotid, COM2, "\033[%u;%uH\033[K%s█",
        LINE_TERMINAL, 3,
        cmd_buffer
    );
}

static void ui_execute_command(int iotid, int traintid, char *cmd_buffer, unsigned int cmd_len) {
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
                ui_print_switch(iotid, (unsigned int) code, (char) direction);
            }
            break;
    }
}

static void ui_keyboard_task() {
    unsigned int cmd_len = 0;
    char cmd_buffer[CMD_BUFFER_SIZE];

    int iotid = WhoIs(SERVER_NAME_IO);
    int traintid = WhoIs(SERVER_NAME_TMS);
    for (;;) {
        char in = (char) Getc(iotid, COM2);
        switch (in) {
            case '\r':                         // execute command if get "ENTER"
                if (cmd_len != 0) {
                    if (cmd_buffer[0] == 'q') {
                        Printf(iotid, COM2, "\033[%u;%uH", LINE_DEBUG, 1);
                        TrainExit(traintid);
                        ShutdownIOServer();
                        Shutdown();
                    }
                    ui_execute_command(iotid, traintid, cmd_buffer, cmd_len);
                    cmd_len = 0;
                    ui_print_terminal(iotid, cmd_buffer, cmd_len);
                }
                break;
            case '\b':                         // delete character if get "BACKSPACE"
                if (cmd_len != 0) {
                    cmd_len -= 1;
                    ui_print_terminal(iotid, cmd_buffer, cmd_len);
                }
                break;
            default:                           // otherwise just store the character
                if (cmd_len < CMD_BUFFER_SIZE) {
                    cmd_buffer[cmd_len] = in;
                    cmd_len += 1;
                    ui_print_terminal(iotid, cmd_buffer, cmd_len);
                }
                break;
        }

    }
    Exit();
}

static void ui_clock_init(UIClock *clock) {
    clock->minute = 0;
    clock->second = 0;
    clock->decisecond = 0;
}

static void ui_clock_update(UIClock *clock) {
    if (clock->decisecond < 9) {
        clock->decisecond++;
        return;
    }
    clock->decisecond = 0;
    if (clock->second < 59) {
        clock->second++;
        return;
    }
    clock->second = 0;
    clock->minute++;
}

static void ui_clock_display(int iotid, UIClock *clock) {
    Printf(iotid, COM2, "\033[%u;%uH\033[K%04u:%02u.%u",
        LINE_TIME, 7,
        clock->minute,
        clock->second,
        clock->decisecond
    );
}

static void ui_clock_task() {
    UIClock clock;
    ui_clock_init(&clock);
    int clocktid = WhoIs(SERVER_NAME_CLOCK);
    int iotid = WhoIs(SERVER_NAME_IO);
    for (;;) {
        Delay(clocktid, CLOCK_PRECISION/10);
        ui_clock_update(&clock);
        ui_clock_display(iotid, &clock);
    }
    Exit();
}

/*static void ui_sensor_task() {*/
    /*activetrainsensorlist active_list; active_list.size = 0;*/
    /*activetrainsensorlist output_list; output_list.size = 0;*/

    /*int clocktid = whois(clock_server_name);*/
    /*int iotid = whois(io_server_name);*/
    /*int traintid = whois(trainset_server_name);*/

    /*for (;;) {*/
        /*delay(clocktid, sensor_read_interval/10);*/
        /*active_list = trainset_sensor_readall(traintid);*/
        /*for (uint32_t i = 0; i < active_list.size; i++) {*/
            /*if (output_list.size < sensor_list_size) {*/
                /*output_list.size += 1;*/
            /*}*/
            /*for (int t = sensor_list_size - 1; t > 0; t--) {*/
                /*output_list.sensors[t] = output_list.sensors[t - 1];*/
            /*}*/
            /*output_list.sensors[0] = active_list.sensors[i];*/
        /*}*/
        /*for (uint32_t i = 0; i < output_list.size; i++) {*/
            /*printf(iotid, com2, "\033[%d;%dh\033[k%c%u",*/
                /*line_sensor_start + i, 3,*/
                /*output_list.sensors[i].module,*/
                /*output_list.sensors[i].id*/
            /*);*/
        /*};*/
    /*}*/
    /*exit();*/
/*}*/

void ui_server_root_task() {
    int iotid = WhoIs(SERVER_NAME_IO);
    // Clear the screen
    Printf(iotid, COM2, "\033[2J");
    // Hide the cursor
    Printf(iotid, COM2, "\033[?25l");
    // Initialize the interface
    ui_print_interface(iotid);

    Create(PRIORITY_SERVER_UI, &ui_keyboard_task);
    Create(PRIORITY_SERVER_UI, &ui_clock_task);
    /*Create(PRIORITY_SERVER_UI, &ui_sensor_task);*/
    Exit();
}

void CreateUIServer() {
    Create(PRIORITY_SERVER_UI, &ui_server_root_task);
}
