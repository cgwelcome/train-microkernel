#include <ts7200.h>
#include <user/tasks.h>
#include <user/io.h>

void print(char *str) {
    while (*str) {
        Putc(0, COM2, *str);
        str++;
    }
}

void task_main() {
    print("Hello World");
    Exit();
}
