#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <user/io.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/queue.h>

#define BUFFER_SIZE 256

void io_perf_test() {
    int servertid = WhoIs(IO_SERVER_NAME);
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = Getc(servertid, COM2);
    }
    for (int i = 0; i < BUFFER_SIZE; i++) {
        Putc(servertid, COM2, buffer[i]);
    }
    Exit();
}

void io_getc_test() {
    int servertid = WhoIs(IO_SERVER_NAME);
    for (;;) {
        int c = Getc(servertid, COM2);
        Putc(servertid, COM2, (char)c);
    }
    Exit();
}

void io_putc_test() {
    int servertid = WhoIs(IO_SERVER_NAME);
    for (int i = 0; i < 10; i++) {
        Putc(servertid, COM2, (char)('0'+i));
    }
    Exit();
}

void io_test_root_task() {
    CreateNameServer(4000);
    CreateIOServer(3000, 3000, 3000);
    // Create(3000, &io_putc_test);
    Create(3000, &io_getc_test);
    // Create(3000, &io_perf_test);
    CreateIdleTask(1);
    Exit();
}
