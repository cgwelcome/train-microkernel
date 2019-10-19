#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <user/io.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/queue.h>

void io_perf_test() {
    int servertid = WhoIs(COM2_SERVER_NAME);
    Queue queue;
    queue_init(&queue);
    for (int i = 0; i < 10; i++) {
        int c = Getc(servertid, COM2);
        queue_push(&queue, c);
    }
    for (int i = 0; i < 10; i++) {
        Putc(servertid, COM2, (char)queue_pop(&queue));
    }
    Exit();
}

void io_getc_test() {
    int servertid = WhoIs(COM2_SERVER_NAME);
    for (;;) {
        int c = Getc(servertid, COM2);
        Putc(servertid, COM2, (char)c);
    }
    Exit();
}

void io_putc_test() {
    int servertid = WhoIs(COM2_SERVER_NAME);
    for (int i = 0; i < 10; i++) {
        Putc(servertid, COM2, (char)('0'+i));
    }
    Exit();
}

void io_test_root_task() {
    CreateNameServer(4000);
    CreateIOServer(3000, COM2);
    /*Create(3000, &io_putc_test);*/
    Create(3000, &io_getc_test);
    /*Create(3000, &io_perf_test);*/
    CreateIdleTask(1);
    Exit();
}
