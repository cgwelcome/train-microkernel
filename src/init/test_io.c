#include <kernel.h>
#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <user/io.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/queue.h>

#define BUFFER_SIZE 256

void io_perf_test() {
    int servertid = WhoIs(SERVER_NAME_IO);
    Printf(servertid, COM2, "IO server performance test start at: %d\r\n", MyTid());
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        Getc(servertid, COM2, &buffer[i]);
    }
    for (int i = 0; i < BUFFER_SIZE; i++) {
        Putc(servertid, COM2, buffer[i]);
    }
    Printf(servertid, COM2, "IO server performance test done.\r\n");
    Exit();
}

void io_getc_test() {
    int servertid = WhoIs(SERVER_NAME_IO);
    Printf(servertid, COM2, "IO server getc test start at: %d\r\n", MyTid());
    char c;
    for (;;) {
        Getc(servertid, COM2, &c);
        if (c == '\r') break;
        Putc(servertid, COM2, c);
    }
    Printf(servertid, COM2, "IO server getc test done.\r\n");
    Exit();
}

void io_putc_test() {
    int servertid = WhoIs(SERVER_NAME_IO);
    Printf(servertid, COM2, "IO server putc test start at : %d\r\n", MyTid());
    for (int i = 0; i < 10; i++) {
        Putc(servertid, COM2, (char)('0'+i));
    }
    Printf(servertid, COM2, "IO server putc test done.\r\n");
    Exit();
}

void io_race_test() {
    int servertid = WhoIs(SERVER_NAME_IO);
    for (int i = 0; i < 9; i++) {
        Printf(servertid, COM2, "This is line %d printed by task %d.\r\n", i + 1, MyTid());
    }
    Exit();
}

void io_test_root_task() {
    CreateIOServer();

    int servertid = WhoIs(SERVER_NAME_IO);
    Printf(servertid, COM2, "\033[2J");

    Create(1000, &io_race_test);
    Create(2000, &io_race_test);
    Create(3000, &io_race_test);
    // Create(3000, &io_putc_test);
    // Create(3000, &io_getc_test);
    // Create(3000, &io_perf_test);
    CreateIdleTask();
    Exit();
}
