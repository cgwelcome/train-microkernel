#include <executable.h>
#include <kernel.h>
#include <server/io.h>
#include <server/taxi.h>
#include <server/ui.h>
#include <train/track.h>
#include <user/io.h>
#include <user/clock.h>
#include <user/name.h>
#include <user/tasks.h>
#include <user/train.h>
#include <user/ui.h>

#include <stdlib.h>

// TODO: is there a better way to store these global variables?
static int io_tid;
static int train_tid;

// NOTE: these structures must be read only in this file!
extern Track singleton_track;
extern const uint32_t train_ids[TRAIN_COUNT];

static uint8_t is_train(uint32_t train_id) {
    for (uint32_t i = 0; i < TRAIN_COUNT; i++) {
        if (train_ids[i] == train_id) return 1;
    }
    return 0;
}

static bool is_speed(uint32_t speed) {
    return (speed == 0) || (10 <= speed && speed <= 14);
}

static bool is_switch(uint32_t switch_id) {
    return (switch_id > 0 && switch_id < 19) || (switch_id > 0x98 && switch_id < 0x9D);
}

static int cmd_init(int nargc, char **nargv) {
    if (nargc == 3 && !strcmp(nargv[1], "track")) {
        char *track = nargv[2];
        if (!strcmp(track, "a") || !strcmp(track, "A")) {
            TrainInitTrack(train_tid, TRAIN_TRACK_A);
            return 0;
        }
        if (!strcmp(track, "b") || !strcmp(track, "B")) {
            TrainInitTrack(train_tid, TRAIN_TRACK_B);
            return 0;
        }
        PrintWarning(io_tid, "invalid track");
        return 1;
    }
    if (nargc == 4 && !strcmp(nargv[1], "train")) {
        if (!singleton_track.inited) {
            PrintWarning(io_tid, "initialize track first!");
            return 1;
        }
        uint32_t train_id = (uint32_t)atoi(nargv[2]);
        TrackNode *node   = track_find_node_by_name(&singleton_track, nargv[3]);
        if (node != NULL && is_train(train_id)) {
            TrainInitTrain(train_tid, train_id, node);
            return 0;
        }
        PrintWarning(io_tid, "invalid train id or node name");
        return 1;
    }
    PrintWarning(io_tid, "usage: init track track_name | init train id node");
    return 1;
}

static int cmd_tr(int nargc, char **nargv) {
    if (nargc != 3) {
        PrintWarning(io_tid, "usage: tr train_id speed");
        return 1;
    }
    uint32_t train_id = (uint32_t)atoi(nargv[1]);
    uint32_t speed = (uint32_t)atoi(nargv[2]);
    if (is_train(train_id) && is_speed(speed)) {
        TrainSpeed(train_tid, train_id, speed);
        return 0;
    }
    PrintWarning(io_tid, "invalid train id or speed");
    return 1;
}

static int cmd_rv(int nargs, char **nargv) {
    if (nargs != 2) {
        PrintWarning(io_tid, "usage: rv train_id");
        return 1;
    }
    uint32_t train_id = (uint32_t)atoi(nargv[1]);
    if (is_train(train_id)) {
        TrainReverse(train_tid, train_id);
        return 0;
    }
    PrintWarning(io_tid, "invalid train id");
    return 1;
}

static int cmd_sw(int nargc, char **nargv) {
    if (nargc != 3) {
        PrintWarning(io_tid, "usage: sw switch_id direction");
        return 1;
    }
    uint32_t switch_id = (uint32_t)atoi(nargv[1]);
    char *direction = nargv[2];
    if (!is_switch(switch_id)) {
        return 1;
    }
    if (!strcmp(direction, "c") || !strcmp(direction, "C")) {
        TrainSwitch(train_tid, switch_id, DIR_CURVED);
        return 0;
    }
    if (!strcmp(direction, "s") || !strcmp(direction, "S")) {
        TrainSwitch(train_tid, switch_id, DIR_STRAIGHT);
        return 0;
    }
    PrintWarning(io_tid, "invalid direction");
    return 1;
}

static int cmd_mv(int nargc, char **nargv) {
    if (nargc != 3 && nargc != 4) {
        PrintWarning(io_tid, "usage: mv train_id node [offset]");
        return 1;
    }
    uint32_t train_id = (uint32_t)atoi(nargv[1]);
    TrackNode *node   = track_find_node_by_name(&singleton_track, nargv[2]);
    int32_t offset = 0;
    if (nargc == 4) {
        offset = atoi(nargv[3]);
    }
    if (node != NULL && is_train(train_id)) {
        TrainMove(train_tid, train_id, 10, node, offset);
        return 0;
    }
    PrintWarning(io_tid, "invalid train id or node name");
    return 1;
}

static int cmd_demo(int nargc, char **nargv) {
    (void)nargc;
    (void)nargv;
    CreateTaxiSystem();
    return 0;
}

static int cmd_clear(int nargc, char **nargv) {
    (void)nargc;
    (void)nargv;
    Printf(io_tid, COM2, "\033[s\033[%u;%uH\033[K\033[u", LINE_WARNING, 1);   // clear warning
    Printf(io_tid, COM2, "\033[s\033[%u;%uH\033[J\033[u", LINE_LOG_START, 1); // clear log region
    return 0;
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
} exec_table[] = {
    { "helloworld",      exec_helloworld        },
    { "argv",            exec_argv              },
    { "model",           exec_build_model       },
    { "setup",           exec_setup             },
    { "searchpath",      exec_search_path       },
    { "searchallpath",   exec_search_allpath    },
    { "reverse",         exec_reverse           },
    { "collision",       exec_collision         },
    { "position",        exec_position          },
    { "routing",         exec_routing           },
    { "trackb",          exec_trackb            },
    { NULL,              NULL                   },
};

static int cmd_exec(int nargc, char **nargv) {
    if (nargc < 2) {
        PrintWarning(io_tid, "usage: exec name [arguments]");
        return 1;
    }

    for (uint32_t i = 0; exec_table[i].name; i ++) {
        if (!strcmp(nargv[1], exec_table[i].name)) {
            exec_table[i].func(nargc-1, nargv+1);
            return 0;
        }
    }
    PrintWarning(io_tid, "executable not found");
    return 1;
}

static struct {
    const char *name;
    int (*func)(int nargc, char **nargv);
} cmd_table[] = {
    { "init",   cmd_init  },
    { "tr",     cmd_tr    },
    { "rv",     cmd_rv    },
    { "sw",     cmd_sw    },
    { "mv",     cmd_mv    },
    { "demo",   cmd_demo  },
    { "clear",  cmd_clear },
    { "q",      cmd_quit  },
    { "quit",   cmd_quit  },
    { "exec",   cmd_exec  },
    { NULL,     NULL      },
};

static int dispatch_command(char *cmd) {
    char *word;
    char *nargv[MAX_NUM_ARGS];
    int nargc = 0;

    for (word = strtok(cmd, " \t"); word != NULL; word = strtok(NULL, " \t")) {
        if (nargc >= MAX_NUM_ARGS) {
            PrintWarning(io_tid, "maximum arguments exceeded");
            return 1;
        }
        nargv[nargc++] = word;
    }

    if (nargc == 0) return 0; // no input from user
    for (uint32_t i = 0; cmd_table[i].name; i ++) {
        if (!strcmp(nargv[0], cmd_table[i].name)) {
            return cmd_table[i].func(nargc, nargv);
        }
    }
    PrintWarning(io_tid, "command not found");
    return 1;
}

static void ui_clock_task() {
    int io_tid = WhoIs(SERVER_NAME_IO);
    int clock_tid = WhoIs(SERVER_NAME_CLOCK);
    for (;;) {
        Delay(clock_tid, CLOCK_PRECISION/10);
        PrintTime(io_tid);
    }
    Exit();
}

static void ui_keyboard_task() {
    uint32_t cmd_len = 0;
    char cmd_buffer[CMD_BUFFER_SIZE];

    io_tid = WhoIs(SERVER_NAME_IO);
    train_tid = WhoIs(SERVER_NAME_TRAIN);

    char in;
    for (;;) {
        Getc(io_tid, COM2, &in);
        switch (in) {
            case '\r':                         // execute command if get "ENTER"
                dispatch_command(cmd_buffer);
                cmd_len = 0;
                cmd_buffer[cmd_len] = '\0';
                PrintTerminal(io_tid, cmd_buffer);
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
