#include <kernel.h>
#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <user/io.h>
#include <user/tasks.h>
#include <user/name.h>

void ctxcom_read_and_write_test() {
    int iotid = WhoIs(SERVER_NAME_IO);
    unsigned int speed = 10;
    Putc(iotid, COM1, (char)speed);
    Putc(iotid, COM1, 78);
    for (;;) {
        char command;
        Getc(iotid, COM2, &command);
        if (command == 'w' && speed < 14) {
            speed++;
            Putc(iotid, COM2, (char)('a'-1+speed));
            Putc(iotid, COM1, (char)speed);
            Putc(iotid, COM1, 78);
        }
        if (command == 's' && speed > 0) {
            speed--;
            Putc(iotid, COM2, (char)('a'-1+speed));
            Putc(iotid, COM1, (char)speed);
            Putc(iotid, COM1, 78);
        }
        Putc(iotid, COM1, 0x80 + 5);
        Printf(iotid, COM2, "\r\n");
        for (int i = 0; i < 10; i++) {
            char sensor;
            Getc(iotid, COM1, &sensor);
            Printf(iotid, COM2, "%02x", (int)sensor);
        }

    }
    Exit();
}

void ctxcom_write_test() {
    int iotid = WhoIs(SERVER_NAME_IO);
    unsigned int speed = 5;
    Putc(iotid, COM1, (char)speed);
    Putc(iotid, COM1, 78);
    char command;
    for (;;) {
        Getc(iotid, COM2, &command);
        if (command == 'w' && speed < 14) {
            speed++;
            Putc(iotid, COM2, (char)('a'-1+speed));
            Putc(iotid, COM1, (char)speed);
            Putc(iotid, COM1, 78);
        }
        if (command == 's' && speed > 1) {
            speed--;
            Putc(iotid, COM2, (char)('a'-1+speed));
            Putc(iotid, COM1, (char)speed);
            Putc(iotid, COM1, 78);
        }
        Putc(iotid, COM2, (char)command);
    }
    Exit();
}

void ctxcom_write_single_test() {
    int servertid = WhoIs(SERVER_NAME_IO);
    Putc(servertid, COM1, 5);
    Putc(servertid, COM1, 78);
    Exit();
}

void ctxcom_test_root_task() {
    CreateNameServer();
    CreateIOServer();
    Create(2000, &ctxcom_read_and_write_test);
    CreateIdleTask();
    Exit();
}
