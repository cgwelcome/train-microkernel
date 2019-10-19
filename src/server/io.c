#include <event.h>
#include <hardware/uart.h>
#include <server/idle.h>
#include <server/io.h>
#include <user/event.h>
#include <user/ipc.h>
#include <user/name.h>
#include <user/tasks.h>
#include <utils/queue.h>

static IOChannel com1iochannel;
static IOChannel com2iochannel;

static void comserver_initiochannel(int channel) {
	IOChannel *iochannel;
	switch (channel) {
		case COM1:
			iochannel = &com1iochannel;
			iochannel->ctsflag = IO_CTSCOMPLETED;
			break;
		case COM2:
			iochannel = &com2iochannel;
			break;
		default:
			return;
			break;
	}
	queue_init(&iochannel->recvqueue);
	queue_init(&iochannel->sendqueue);
	iochannel->id = channel;
	iochannel->recvflag = IO_RECVUP;
	iochannel->transmitflag = IO_TRANSMITUP;
}

static void com1server_updatesend(IOChannel *iochannel) {
	if (iochannel->transmitflag == IO_TRANSMITUP
			&& iochannel->ctsflag == IO_CTSCOMPLETED
			&& queue_size(&iochannel->sendqueue) > 0) {

		uart_putc(iochannel->id, (char)queue_pop(&iochannel->sendqueue));
		iochannel->ctsflag = IO_CTSINIT;
		iochannel->transmitflag = IO_TRANSMITDOWN;
		uart_enableintr(iochannel->id, MSIEN_MASK | TIEN_MASK);
	}
}

static void com2server_updatesend(IOChannel *iochannel) {
	if (iochannel->transmitflag == IO_TRANSMITUP
			&& queue_size(&iochannel->sendqueue) > 0) {

		uart_putc(iochannel->id, (char)queue_pop(&iochannel->sendqueue));
		iochannel->transmitflag = IO_TRANSMITDOWN;
		uart_enableintr(iochannel->id, TIEN_MASK);
	}
}

static void comserver_updaterecv(IOChannel *iochannel) {
	if (iochannel->recvflag == IO_RECVDOWN
			&& queue_size(&iochannel->recvqueue) > 0) {

		int c = uart_getc(iochannel->id);
		while (queue_size(&iochannel->recvqueue) > 0) {
			int tid = queue_pop(&iochannel->recvqueue);
			Reply(tid, (char *)&c, sizeof(c));
		}
		iochannel->recvflag = IO_RECVUP;
	}
}

static void comserver_updateiochannel(IOChannel *iochannel) {
	switch (iochannel->id) {
		case COM1:
			com1server_updatesend(iochannel);
			break;
		case COM2:
			com2server_updatesend(iochannel);
			break;
	}
	comserver_updaterecv(iochannel);
}

static void comserver_getc(IOChannel *iochannel, int tid) {
	queue_push(&iochannel->recvqueue, tid);
	uart_enableintr(iochannel->id, RIEN_MASK);
}

static void comserver_putc(IOChannel *iochannel, uint8_t c) {
	queue_push(&iochannel->sendqueue, c);
	comserver_updateiochannel(iochannel);
}

static void comserver_updateflag(IOChannel *iochannel, uint32_t flag) {
	if (flag & MIS_MASK) {
		uart_clearmsintr(iochannel->id);
		int uartstatus = uart_readflag(iochannel->id);
		if (!(uartstatus & CTS_MASK) && iochannel->ctsflag == IO_CTSINIT) {
			iochannel->ctsflag = IO_CTSDOWN;
		}
		else if (uartstatus & CTS_MASK && iochannel->ctsflag == IO_CTSDOWN) {
			iochannel->ctsflag = IO_CTSCOMPLETED;
			uart_disableintr(iochannel->id, MSIEN_MASK);
		}
	}
	if (flag & RIS_MASK) {
		iochannel->recvflag = IO_RECVDOWN;
		uart_disableintr(iochannel->id, RTIEN_MASK);
	}
	if (flag & TIS_MASK) {
		iochannel->transmitflag = IO_TRANSMITUP;
		uart_disableintr(iochannel->id, TIEN_MASK);
	}
}

static void comserver_task(IOChannel *iochannel) {
	int tid; IORequest request;

	for (;;) {
		Receive(&tid, (char *)&request, sizeof(request));
		switch (request.type) {
			case IO_REQUEST_INT_UART:
				comserver_updateflag(iochannel, request.data);
				comserver_updateiochannel(iochannel);
				Reply(tid, NULL, 0);
				break;
			case IO_REQUEST_PUTC:
				comserver_putc(iochannel, (uint8_t)request.data);
				Reply(tid, NULL, 0);
				break;
			case IO_REQUEST_GETC:
				comserver_getc(iochannel, tid);
				break;
			default:
				break;
		}
	}
}

void com1server_task() {
	uart_setbitconfig(COM1, WLEN_MASK | STP2_MASK);
	uart_setspeed(COM1, 2400);
	comserver_initiochannel(COM1);
	uart_enableintr(COM1, UARTEN_MASK);
	RegisterAs(COM1_SERVER_NAME);
	comserver_task(&com1iochannel);
}

void com2server_task() {
	uart_setbitconfig(COM2, WLEN_MASK);
	uart_setspeed(COM2, 115200);
	comserver_initiochannel(COM2);
	uart_enableintr(COM2, UARTEN_MASK);
	RegisterAs(COM2_SERVER_NAME);
	comserver_task(&com2iochannel);
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
