#include <event.h>
#include <hardware/uart.h>
#include <server/idle.h>
#include <server/io.h>
#include <user/event.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/queue.h>

static IOStatus com1status;
static IOStatus com2status;

static void comserver_init(int channel) {
	IOStatus *iostatus;
	switch (channel) {
		case COM1:
			iostatus = &com1status;
			iostatus->ctsflag = IO_CTSCOMPLETED;
			break;
		case COM2:
			iostatus = &com2status;
			break;
		default:
			return;
			break;
	}
	queue_init(&iostatus->recvqueue);
	queue_init(&iostatus->sendqueue);
	iostatus->recvflag = IO_RECVUP;
	iostatus->transmitflag = IO_TRANSMITUP;
}

static void comserver_updatequeue(int channel) {
	IOStatus *iostatus;
	uint8_t sendready;
	uint8_t sendintr;

	switch (channel) {
		case COM1:
			iostatus = &com1status;
			sendready = (iostatus->transmitflag == IO_TRANSMITUP && iostatus->ctsflag == IO_CTSCOMPLETED) ? 1 : 0;
			sendintr = TIEN_MASK | MSIEN_MASK;
			break;
		case COM2:
			iostatus = &com2status;
			sendready = (iostatus->transmitflag == IO_TRANSMITUP) ? 1 : 0;
			sendintr = TIEN_MASK;
			break;
		default:
			return;
			break;
	}
	if (iostatus->recvflag == IO_RECVDOWN && queue_size(&iostatus->recvqueue) > 0) {
		int c = uart_getc(channel);
		while (queue_size(&iostatus->recvqueue) > 0) {
			int tid = queue_pop(&iostatus->recvqueue);
			Reply(tid, (char *)&c, sizeof(c));
		}
		iostatus->recvflag = IO_RECVUP;
	}
	else if (sendready && queue_size(&iostatus->sendqueue) > 0) {
		uart_putc(channel, (char)queue_pop(&iostatus->sendqueue));
		iostatus->ctsflag = IO_CTSINIT;
		iostatus->transmitflag = IO_TRANSMITDOWN;
		uart_enableintr(channel, sendintr);
	}
	else if (!sendready) {
		uart_enableintr(channel, sendintr);
	}
}

static void comserver_getc(int channel, int tid) {
	switch (channel) {
		case COM1:
			queue_push(&com1status.recvqueue, tid);
			break;
		case COM2:
			queue_push(&com2status.recvqueue, tid);
			break;
		default:
			break;
	}
	uart_enableintr(channel, RIEN_MASK);
}

static int comserver_putc(int channel, uint8_t c) {
	switch (channel) {
		case COM1:
			queue_push(&com1status.sendqueue, c);
			break;
		case COM2:
			queue_push(&com2status.sendqueue, c);
			break;
		default:
			return -1;
			break;
	}
	comserver_updatequeue(channel);
	return 0;
}

static void comserver_updateflag(int channel, uint32_t flag) {
	IOStatus *iostatus;
	switch (channel) {
		case COM1:
			iostatus = &com1status;
			break;
		case COM2:
			iostatus = &com2status;
			break;
		default:
			return;
			break;
	}
	if (flag & MIS_MASK) {
		int uartstatus = uart_readflag(channel);
		if (!(uartstatus & CTS_MASK) && iostatus->ctsflag == IO_CTSINIT) {
			iostatus->ctsflag = IO_CTSDOWN;
		}
		else if (uartstatus & CTS_MASK && iostatus->ctsflag == IO_CTSDOWN) {
			iostatus->ctsflag = IO_CTSCOMPLETED;
		}
		uart_clearmsintr(channel);
		uart_disableintr(channel, MSIEN_MASK);
	}
	if (flag & RIS_MASK) {
		iostatus->recvflag = IO_RECVDOWN;
		uart_disableintr(channel, RTIEN_MASK);
	}
	if (flag & TIS_MASK) {
		iostatus->transmitflag = IO_TRANSMITUP;
		uart_disableintr(channel, TIEN_MASK);
	}
}

static void comserver_task(int channel) {
	int tid;
    IORequest request;

	for (;;) {
		Receive(&tid, (char *)&request, sizeof(request));
		switch (request.type) {
			case IO_REQUEST_INT_UART:
				comserver_updateflag(channel, request.data);
				comserver_updatequeue(channel);
				Reply(tid, NULL, 0);
				break;
			case IO_REQUEST_PUTC:
				comserver_putc(channel, (uint8_t)request.data);
				Reply(tid, NULL, 0);
				break;
			case IO_REQUEST_GETC:
				comserver_getc(channel, tid);
				break;
			default:
				break;
		}
	}
}

void com1server_task() {
	uart_setbitconfig(COM1, WLEN_MASK | STP2_MASK);
	uart_setspeed(COM1, 2400);
	comserver_init(COM1);
	uart_enableintr(COM1, UARTEN_MASK);
	RegisterAs(COM1_SERVER_NAME);
	comserver_task(COM1);
}

void com2server_task() {
	uart_setbitconfig(COM2, WLEN_MASK);
	uart_setspeed(COM2, 115200);
	comserver_init(COM2);
	uart_enableintr(COM2, UARTEN_MASK);
	RegisterAs(COM2_SERVER_NAME);
	comserver_task(COM2);
}

static void comnotifier_await(int channel, int servertid) {
	IORequest request;
	int event;
	switch (channel) {
		case COM1:
			event = INT_UART1;
			break;
		case COM2:
			event = INT_UART2;
			break;
		default:
			return;
			break;
	}
	for (;;) {
		int uartintr = AwaitEvent(event);
		request.type = IO_REQUEST_INT_UART;
		request.data = (uint32_t) uartintr;
		Send(servertid, (char *)&request, sizeof(request), NULL, 0);
	}
}

void com1notifier_task() {
	comnotifier_await(COM1, WhoIs(COM1_SERVER_NAME));
}

void com2notifier_task() {
	comnotifier_await(COM2, WhoIs(COM2_SERVER_NAME));
}

int CreateIOServer(uint32_t priority, int channel) {
	int servertid;
	switch (channel) {
		case COM1:
			servertid = Create(priority, &com1server_task);
			Create(priority-10, &com1notifier_task);
			break;
		case COM2:
			servertid = Create(priority, &com2server_task);
			Create(priority-10, &com2notifier_task);
			break;
		default:
			servertid = -1;
			break;
	}
	return servertid;
}
