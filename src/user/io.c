#include <server/io.h>
#include <user/ipc.h>
#include <user/name.h>

int Getc(int tid, int uart) {
    (void)uart;
    IORequest request = {
        .type = IO_REQUEST_GETC,
    };
    int response;
    int status = Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response));
    if (status < 0) return -3;
    return response;
}

int Putc(int tid, int uart, char c) {
    (void)uart;
    IORequest request = {
        .type = IO_REQUEST_PUTC,
        .data = c,
    };
    int status = Send(tid, (char *)&request, sizeof(request), NULL, 0);
    if (status < 0) return -3;
    return 0;
}
