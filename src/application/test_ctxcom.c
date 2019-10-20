#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <user/io.h>
#include <user/tasks.h>
#include <user/name.h>

void ctxcom_read_and_write_test() {
    int iotid = WhoIs(IO_SERVER_NAME);
    unsigned int speed = 10;
    Putc(iotid, COM1, (char)speed);
    Putc(iotid, COM1, 78);
    for (;;) {
        int command = Getc(iotid, COM2);
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
        Putc(iotid, COM1, 0x80 + 5);
        Printf(iotid, COM2, "\n\r");
        for (int i = 0; i < 10; i++) {
            Printf(iotid, COM2, "%02x", Getc(iotid, COM1));
        }

    }
    Exit();
}

void ctxcom_write_test() {
    int iotid = WhoIs(IO_SERVER_NAME);
    unsigned int speed = 5;
    Putc(iotid, COM1, (char)speed);
    Putc(iotid, COM1, 78);
    for (;;) {
        int command = Getc(iotid, COM2);
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
    int servertid = WhoIs(IO_SERVER_NAME);
    Putc(servertid, COM1, 5);
    Putc(servertid, COM1, 78);
    Exit();
}

void ctxcom_test_root_task() {
    CreateNameServer(4000);
    CreateIOServer(3000, 2500, 2500);
    Create(2000, &ctxcom_read_and_write_test);
    CreateIdleTask(1);
    Exit();
}
