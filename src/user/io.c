#include <server/io.h>
#include <user/ipc.h>
#include <user/name.h>

int Getc(int tid, int uart) {
	int servertid;
	switch (uart) {
		case COM1:
			servertid = WhoIs(COM1_SERVER_NAME);
			break;
		case COM2:
			servertid = WhoIs(COM2_SERVER_NAME);
			break;
		default:
			return -2;
			break;
	}
	IORequest request = {
		.type = IO_REQUEST_GETC,
	};
	int response;
	if (tid != servertid) return -1;
    int status = Send(tid, (char *)&request, sizeof(request), (char *)&response, sizeof(response));
    if (status < 0) return -3;
	return response;
}

int Putc(int tid, int uart, char c) {
	int servertid;
	switch (uart) {
		case COM1:
			servertid = WhoIs(COM1_SERVER_NAME);
			break;
		case COM2:
			servertid = WhoIs(COM2_SERVER_NAME);
			break;
		default:
			return -2;
			break;
	}
	if (tid != servertid) return -1;
	IORequest request = {
		.type = IO_REQUEST_PUTC,
		.data = c,
	};
    int status = Send(tid, (char *)&request, sizeof(request), NULL, 0);
    if (status < 0) return -3;
	return 0;
}
