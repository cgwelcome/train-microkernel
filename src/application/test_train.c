#include <server/idle.h>
#include <server/io.h>
#include <server/name.h>
#include <user/io.h>
#include <user/tasks.h>
#include <user/name.h>
#include <utils/bwio.h>

void train_sensors_test() {
    int io_server_tid = WhoIs(IO_SERVER_NAME);
    unsigned int speed = 10;
    Putc(io_server_tid, COM1, (char)speed);
    Putc(io_server_tid, COM1, 78);
    for (;;) {
        int command = Getc(io_server_tid, COM2);
        if (command == 'w' && speed < 14) {
            speed++;
            Putc(io_server_tid, COM2, (char)('a'-1+speed));
            Putc(io_server_tid, COM1, (char)speed);
            Putc(io_server_tid, COM1, 78);
        }
        if (command == 's' && speed > 1) {
            speed--;
            Putc(io_server_tid, COM2, (char)('a'-1+speed));
            Putc(io_server_tid, COM1, (char)speed);
            Putc(io_server_tid, COM1, 78);
        }
        Putc(io_server_tid, COM1, 0x80 + 5);
        Printf(io_server_tid, COM2, "\n\r");
        for (int i = 0; i < 10; i++) {
            Printf(io_server_tid, COM2, "%02x", Getc(io_server_tid, COM1));
        }

    }
    Exit();
}



void train_game_test() {
    int io_server_tid = WhoIs(IO_SERVER_NAME);
    unsigned int speed = 5;
    Putc(io_server_tid, COM1, (char)speed);
    Putc(io_server_tid, COM1, 78);
    for (;;) {
        int command = Getc(io_server_tid, COM2);
        if (command == 'w' && speed < 14) {
            speed++;
            Putc(io_server_tid, COM2, (char)('a'-1+speed));
            Putc(io_server_tid, COM1, (char)speed);
            Putc(io_server_tid, COM1, 78);
        }
        if (command == 's' && speed > 1) {
            speed--;
            Putc(io_server_tid, COM2, (char)('a'-1+speed));
            Putc(io_server_tid, COM1, (char)speed);
            Putc(io_server_tid, COM1, 78);
        }
        Putc(io_server_tid, COM2, (char)command);
    }
    Exit();
}


void train_subcommand_test() {
    int io_server_tid = WhoIs(IO_SERVER_NAME);
    Putc(io_server_tid, COM1, 5);
    Putc(io_server_tid, COM1, 78);
    for (;;) {
        int speed = Getc(io_server_tid, COM2);
        Putc(io_server_tid, COM2, (char)speed);
        Putc(io_server_tid, COM1, (char)(speed-'a'));
        Putc(io_server_tid, COM1, 78);
    }
    Exit();
}

void move_train_test() {
    int io_server_tid = WhoIs(IO_SERVER_NAME);
    Putc(io_server_tid, COM1, 5);
    Putc(io_server_tid, COM1, 78);
    Exit();
}

void train_test_root_task() {
    CreateNameServer(4000);
    CreateIOServer(3000, 3000, 3000);
    Create(3000, &train_sensors_test);
    CreateIdleTask(1);
    Exit();
}
