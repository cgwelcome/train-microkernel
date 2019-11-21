#include <server/io.h>
#include <user/io.h>
#include <user/ipc.h>
#include <user/name.h>
#include <utils/assert.h>

#include <stdio.h>

#define PRINTF_MAX_OUTPUT_SIZE 128

void Getc(int tid, int uart, char *c) {
    IORequest request = {
        .type = IO_REQUEST_GET,
        .uart = uart,
        .data = (uint32_t) c,
        .size = 1,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void Putc(int tid, int uart, char c) {
    IORequest request = {
        .type = IO_REQUEST_PUT,
        .uart = uart,
        .data = (uint32_t) &c,
        .size = 1,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void Getw(int tid, int uart, char *buffer, size_t size) {
    IORequest request = {
        .type = IO_REQUEST_GET,
        .uart = uart,
        .data = (uint32_t) buffer,
        .size = size,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void Putw(int tid, int uart, char *buffer, size_t size) {
    IORequest request = {
        .type = IO_REQUEST_PUT,
        .uart = uart,
        .data = (uint32_t) buffer,
        .size = size,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
}

void Printf(int tid, int uart, char *fmt, ...) {
    va_list va;

    va_start(va, fmt);
    char buffer[PRINTF_MAX_OUTPUT_SIZE];
    size_t size = (size_t) vsnprintf(buffer, PRINTF_MAX_OUTPUT_SIZE, fmt, va);
    Putw(tid, uart, buffer, size);
    va_end(va);
}
