#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <user/io.h>
#include <user/tasks.h>
#include <user/name.h>
#include <utils/bwio.h>

void io_sensors_test() {
    int com1tid = WhoIs(COM1_SERVER_NAME);
    int com2tid = WhoIs(COM2_SERVER_NAME);
    unsigned int speed = 10;
    Putc(com1tid, COM1, (char)speed);
    Putc(com1tid, COM1, 78);
    for (;;) {
        int command = Getc(com2tid, COM2);
        if (command == 'w' && speed < 14) {
            speed++;
            Putc(com2tid, COM2, (char)('a'-1+speed));
            Putc(com1tid, COM1, (char)speed);
            Putc(com1tid, COM1, 78);
        }
        if (command == 's' && speed > 1) {
            speed--;
            Putc(com2tid, COM2, (char)('a'-1+speed));
            Putc(com1tid, COM1, (char)speed);
            Putc(com1tid, COM1, 78);
        }
        Putc(com1tid, COM1, 0x80 + 5);
        bwprintf(COM2, "\n\r");
        for (int i = 0; i < 10; i++) {
            bwprintf(COM2, "%02x", Getc(com1tid, COM1));
        }

    }
    Exit();
}



void io_traingame_test() {
    int com1tid = WhoIs(COM1_SERVER_NAME);
    int com2tid = WhoIs(COM2_SERVER_NAME);
    unsigned int speed = 5;
    Putc(com1tid, COM1, (char)speed);
    Putc(com1tid, COM1, 78);
    for (;;) {
        int command = Getc(com2tid, COM2);
        if (command == 'w' && speed < 14) {
            speed++;
            Putc(com2tid, COM2, (char)('a'-1+speed));
            Putc(com1tid, COM1, (char)speed);
            Putc(com1tid, COM1, 78);
        }
        if (command == 's' && speed > 1) {
            speed--;
            Putc(com2tid, COM2, (char)('a'-1+speed));
            Putc(com1tid, COM1, (char)speed);
            Putc(com1tid, COM1, 78);
        }
        Putc(com2tid, COM2, (char)command);
    }
    Exit();
}


void io_subcommand_test() {
    int com1tid = WhoIs(COM1_SERVER_NAME);
    int com2tid = WhoIs(COM2_SERVER_NAME);
    Putc(com1tid, COM1, 5);
    Putc(com1tid, COM1, 78);
    for (;;) {
        int speed = Getc(com2tid, COM2);
        Putc(com2tid, COM2, (char)speed);
        Putc(com1tid, COM1, (char)(speed-'a'));
        Putc(com1tid, COM1, 78);
    }
    Exit();
}

void io_move_train_test() {
    int servertid = WhoIs(COM1_SERVER_NAME);
    Putc(servertid, COM1, 5);
    Putc(servertid, COM1, 78);
    Exit();
}

void train_test_root_task() {
    CreateNameServer(4000);
    CreateIOServer(3000, COM1);
    CreateIOServer(3000, COM2);
    Create(3000, &io_sensors_test);
    CreateIdleTask(1);
    Exit();
}
