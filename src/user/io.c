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

void Putw(int tid, int uart, int n, char fc, char *bf) {
    char ch;
    char *p = bf;

    while(*p++ && n > 0) n--;
    while(n-- > 0) Putc(tid, uart, fc);
    while((ch = *bf++)) Putc(tid, uart, ch);
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
    char bf[12];
    char ch, lz;
    int w;

    while ((ch = *(fmt++))) {
        if (ch != '%')
            Putc(tid, uart, ch);
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
                Putc(tid, uart, va_arg(va, char));
                break;
            case 's':
                Putw(tid, uart, w, 0, va_arg( va, char* ));
                break;
            case 'u':
                ui2a(va_arg( va, unsigned int ), 10, bf);
                Putw(tid, uart, w, lz, bf);
                break;
            case 'd':
                i2a(va_arg( va, int ), bf);
                Putw(tid, uart, w, lz, bf);
                break;
            case 'x':
                ui2a(va_arg( va, unsigned int ), 16, bf);
                Putw(tid, uart, w, lz, bf);
                break;
            case '%':
                Putc(tid, uart, ch);
                break;
            }
        }
    }
}

void Printf(int tid, int uart, char *fmt, ...) {
    va_list va;

    va_start(va,fmt);
    format(tid, uart, fmt, va);
    va_end(va);
}
