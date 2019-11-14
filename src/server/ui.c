#include <stdlib.h>
#include <kernel.h>
#include <test.h>
#include <server/io.h>
#include <server/ui.h>
#include <user/io.h>
#include <user/clock.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/train.h>
#include <user/ui.h>

static int io_tid;
static int train_tid;

static int is_train(uint32_t train_id) {
    return (train_id > 0 && train_id < 81) ? 1 : 0;
}

static int is_speed(uint32_t speed) {
    return (speed <= 14) ? 1 : 0;
}

static int is_switch(uint32_t switch_id) {
    if ((switch_id > 0 && switch_id < 19) || (switch_id > 0x98 && switch_id < 0x9D)) {
        return 1;
    }
    return 0;
}

static int cmd_init(int nargc, char **nargv) {
    if (nargc != 2) {
        PrintTerminal(io_tid, "usage: init track");
        return 1;
    };
    char *track = nargv[1];

    TrainSwitchOne(train_tid, 6, DIR_STRAIGHT);
    TrainSwitchOne(train_tid, 9, DIR_STRAIGHT);
    TrainSwitchOne(train_tid, 15, DIR_STRAIGHT);
    if (!strcmp(track, "a") || !strcmp(track, "A")) {
        TrainInitTrack(train_tid, TRAIN_TRACK_A);
        return 0;
    }
    if (!strcmp(track, "b") || !strcmp(track, "B")) {
        TrainInitTrack(train_tid, TRAIN_TRACK_B);
        return 0;
    }
    PrintTerminal(io_tid, "invalid track");
    return 1;
}

static int cmd_cv(int nargc, char **nargv) {
    if (nargc != 2) {
        PrintTerminal(io_tid, "usage: cv train_id");
        return 1;
    };
    uint32_t train_id = (uint32_t)atoi(nargv[1]);
    if (is_train(train_id)) {
        TrainStart(train_tid, train_id);
        return 0;
    }
    PrintTerminal(io_tid, "invalid train id");
    return 1;
}

static int cmd_tr(int nargc, char **nargv) {
    if (nargc != 3) {
        PrintTerminal(io_tid, "usage: tr train_id speed");
        return 1;
    };
    uint32_t train_id = (uint32_t)atoi(nargv[1]);
    uint32_t speed = (uint32_t)atoi(nargv[2]);
    if (is_train(train_id) && is_speed(speed)) {
        TrainSetSpeed(train_tid, train_id, speed);
        return 0;
    }
    PrintTerminal(io_tid, "invalid train id/speed");
    return 1;
}

static int cmd_rv(int nargs, char **nargv) {
    if (nargs != 2) {
        PrintTerminal(io_tid, "usage: rv train_id");
        return 1;
    };
    uint32_t train_id = (uint32_t)atoi(nargv[1]);
    if (is_train(train_id)) {
        TrainReverse(train_tid, train_id);
        return 0;
    }
    PrintTerminal(io_tid, "invalid train id");
    return 1;
}

static int cmd_sw(int nargc, char **nargv) {
    if (nargc != 3) {
        PrintTerminal(io_tid, "usage: sw switch_id direction");
        return 1;
    };
    uint32_t switch_id = (uint32_t)atoi(nargv[1]);
    char *direction = nargv[2];
    if (!is_switch(switch_id)) {
        return 1;
    }
    if (!strcmp(direction, "c") || !strcmp(direction, "C")) {
        TrainSwitchOne(train_tid, switch_id, DIR_CURVED);
        return 0;
    }
    if (!strcmp(direction, "s") || !strcmp(direction, "S")) {
        TrainSwitchOne(train_tid, switch_id, DIR_STRAIGHT);
        return 0;
    }
    PrintTerminal(io_tid, "invalid direction");
    return 1;
}

static int cmd_quit(int nargc, char **nargv) {
    (void)nargc;
    (void)nargv;
    TrainExit(train_tid);
    ShutdownIOServer();
    Shutdown();
    return 0;
}

static struct {
    const char *name;
    int (*func)(int argc, char **argv);
} testtable[] = {
    { "helloworld",   helloworld },
    { NULL,           NULL       },
};

static int cmd_test(int nargc, char **nargv) {
    if (nargc < 2) {
        PrintTerminal(io_tid, "usage: test name [arguments]");
        return 1;
    }
    Printf(io_tid, COM2, "\033[%u;%uH", LINE_LOAD, 1);
    Printf(io_tid, COM2, "\033[J", LINE_LOAD, 1);

    for (uint32_t i = 0; testtable[i].name; i ++) {
        if (!strcmp(nargv[1], testtable[i].name)) {
            testtable[i].func(nargc-1, nargv+1);
            return 0;
        }
    }
    PrintTerminal(io_tid, "test not found");
    return 1;
}

static struct {
    const char *name;
    int (*func)(int nargc, char **nargv);
} cmdtable[] = {
    { "init",   cmd_init },
    { "tr",     cmd_tr   },
    { "cv",     cmd_cv   },
    { "rv",     cmd_rv   },
    { "sw",     cmd_sw   },
    { "q",      cmd_quit },
    { "quit",   cmd_quit },
    { "test",   cmd_test },
    { NULL,     NULL     },
};


static int cmd_dispatch(char *cmd) {
    char *word;
    char *nargv[MAX_NUM_ARGS];
    int nargc = 0;

    for (word = strtok(cmd, " \t"); word != NULL; word = strtok(NULL, " \t")) {
        if (nargc >= MAX_NUM_ARGS) {
            PrintTerminal(io_tid, "maximum arguments exceeded");
            return 1;
        }
        nargv[nargc++] = word;
    }
    /** No input from user */
    if (nargc == 0) {
        return 0;
    }
    for (uint32_t i = 0; cmdtable[i].name; i ++) {
        if (!strcmp(nargv[0], cmdtable[i].name)) {
            int returncode = cmdtable[i].func(nargc, nargv);
            return returncode;
        }
    }
    PrintTerminal(io_tid, "command not found");
    return 1;
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
    int returncode;
    uint32_t cmd_len = 0;
    char cmd_buffer[CMD_BUFFER_SIZE];

    io_tid = WhoIs(SERVER_NAME_IO);
    train_tid = WhoIs(SERVER_NAME_TMS);

    for (;;) {
        char in = (char) Getc(io_tid, COM2);
        switch (in) {
            case '\r':                         // execute command if get "ENTER"
                returncode = cmd_dispatch(cmd_buffer);
                cmd_len = 0;
                cmd_buffer[cmd_len] = '\0';
                if (!returncode) PrintTerminal(io_tid, cmd_buffer);
                break;
            case '\b':                         // delete character if get "BACKSPACE"
                if (cmd_len != 0) {
                    cmd_len -= 1;
                    cmd_buffer[cmd_len] = '\0';
                    PrintTerminal(io_tid, cmd_buffer);
                }
                break;
            default:                           // otherwise just store the character
                if (cmd_len < CMD_BUFFER_SIZE-1) {
                    cmd_buffer[cmd_len++] = in;
                    cmd_buffer[cmd_len] = '\0';
                    PrintTerminal(io_tid, cmd_buffer);
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
