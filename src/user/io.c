#include <string.h>
#include <server/io.h>
#include <user/io.h>
#include <user/ipc.h>
#include <user/name.h>

int Getc(int tid, int uart) {
    IORequest request = {
        .type = IO_REQUEST_GETC,
        .uart = uart
    };
    int response;
    int status = Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response));
    if (status < 0) return status;
    return response;
}

int Putc(int tid, int uart, char c) {
    IORequest request = {
        .type = IO_REQUEST_PUTC,
        .uart = uart,
        .data = c,
    };
    int status = Send(tid, (char *)&request, sizeof(request), NULL, 0);
    if (status < 0) return status;
    return 0;
}

int Putw(int tid, int uart, char *buffer) {
    IORequest request = {
        .type = IO_REQUEST_PUTW,
        .uart = uart,
        .data = (uint32_t) buffer
    };
    int status = Send(tid, (char *)&request, sizeof(request), NULL, 0);
    if (status < 0) return status;
    return 0;
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
            while (w--) bf[i++] = lz;

            switch(ch) {
            case 0: return;
            case 'c':
                bf[i++] = va_arg(va, char);
                break;
            case 's':
                strcpy(bf + (i++), va_arg(va, char*));
                break;
            case 'u':
                ui2a(va_arg(va, unsigned int), 10, num);
                strcpy(bf + (i++), num);
                break;
            case 'd':
                i2a(va_arg(va, int), num);
                strcpy(bf + (i++), num);
                break;
            case 'x':
                ui2a(va_arg(va, unsigned int), 16, num);
                strcpy(bf + (i++), num);
                break;
            case '%':
                bf[i++] = ch;
                break;
            }
        }
    }
    bf[i] = '\0';
    Putw(tid, uart, bf);
}

void Printf(int tid, int uart, char *fmt, ...) {
    va_list va;

    va_start(va,fmt);
    format(tid, uart, fmt, va);
    va_end(va);
}
