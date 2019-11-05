#include <server/io.h>
#include <user/io.h>
#include <user/ipc.h>
#include <user/name.h>
#include <utils/assert.h>

int Getc(int tid, int uart) {
    char c;
    IORequest request = {
        .type = IO_REQUEST_GET,
        .uart = uart,
        .data = (uint32_t) &c,
        .size = 1,
    };
    assert(Send(tid, (char *)&request, sizeof(request), NULL, 0) >= 0);
    return (int)c;
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

int a2d(char ch) {
    if(ch >= '0' && ch <= '9') return ch - '0';
    if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if(ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

char a2i(char ch, char **src, int base, int *nump) {
    int num, digit;
    char *p;

    p = *src; num = 0;
    while ( (digit = a2d(ch)) >= 0 ) {
        if (digit > base) break;
        num = num*base + digit;
        ch = *p++;
    }
    *src = p; *nump = num;
    return ch;
}

void ui2a(unsigned int num, unsigned int base, char *bf) {
    int n = 0;
    int dgt;
    unsigned int d = 1;

    while ((num / d) >= base) d *= base;
    while (d != 0) {
        dgt = (int) (num / d);
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0) {
            *bf++ = (char) (dgt + ( dgt < 10 ? '0' : 'a' - 10 ));
            ++n;
        }
    }
    *bf = 0;
}

void i2a(int num, char *bf) {
    if (num < 0) {
        num = -num;
        *bf++ = '-';
    }
    ui2a((unsigned int) num, 10, bf);
}

void bufcpy(char *dst, char *src, int fill_count, char fill_char, int *dst_start) {
    char ch;
    char *p = src;

    while (*p++ && fill_count > 0) fill_count--;
    while (fill_count-- > 0) dst[(*dst_start)++] = fill_char;
    while ((ch = *(src++)))  dst[(*dst_start)++] = ch;
}

void format(int tid, int uart, char *fmt, va_list va) {
    char bf[128], num[32];
    char ch, lz;
    int w;

    int i = 0;
    while ((ch = *(fmt++))) {
        if (ch != '%')
            bf[i++] = ch;
        else {
            lz = ' '; w = 0;
            ch = *(fmt++);
            switch (ch) {
            case '0':
                lz = '0'; ch = *(fmt++);
                __attribute__ ((fallthrough));
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                ch = a2i(ch, &fmt, 10, &w);
                break;
            }
            switch(ch) {
            case 0: return;
            case 'c':
                bf[i++] = va_arg(va, char);
                break;
            case 's':
                bufcpy(bf, va_arg(va, char*), w, lz, &i);
                break;
            case 'u':
                ui2a(va_arg(va, unsigned int), 10, num);
                bufcpy(bf, num, w, lz, &i);
                break;
            case 'd':
                i2a(va_arg(va, int), num);
                bufcpy(bf, num, w, lz, &i);
                break;
            case 'x':
                ui2a(va_arg(va, unsigned int), 16, num);
                bufcpy(bf, num, w, lz, &i);
                break;
            case '%':
                bf[i++] = ch;
                break;
            }
        }
    }
    Putw(tid, uart, bf, (size_t) i);
}

void Printf(int tid, int uart, char *fmt, ...) {
    va_list va;

    va_start(va, fmt);
    format(tid, uart, fmt, va);
    va_end(va);
}
