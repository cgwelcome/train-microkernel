#include <string.h>
#include <kern/ipc.h>
#include <kern/tasks.h>

static size_t msg_copy(Message *dest, Message *source) {
    size_t size = source->len < dest->len ? source->len : dest->len;
    memcpy(dest->array, source->array, size);
    return size;
}

static int ipc_connectable(Task *task) {
    return task->status != UNUSED && task->status != ZOMBIE;
}

static void ipc_recvsend(Task *receiver, Task *sender) {
    *(receiver->send_tid) = sender->tid;
    receiver->tf->r0 = msg_copy(&receiver->recv_msg, &sender->send_msg);
    sender->status = REPLYBLOCKED;
}

void ipc_send(int tid, int recvtid, char *msg, size_t msglen, char *reply, size_t rplen) {
    Task *current_task = task_at(tid);
    Task *recv_task = task_at(recvtid);
    if (!recv_task || !ipc_connectable(recv_task)) {
        current_task->tf->r0 = (uint32_t) -1;
        return;
    }
    current_task->send_msg.array = msg;
    current_task->send_msg.len = msglen;
    current_task->reply_msg.array = reply;
    current_task->reply_msg.len = rplen;

    if (recv_task->status == SENDBLOCKED) {
        ipc_recvsend(recv_task, current_task);
        recv_task->status = READY;
    } else {
        queue_push(&recv_task->send_queue, current_task->tid);
        current_task->status = RECVBLOCKED;
    }
}

void ipc_receive(int tid, int *sendtid, char *msg, size_t msglen) {
    Task *current_task = task_at(tid);

    current_task->send_tid = sendtid;
    current_task->recv_msg.array = msg;
    current_task->recv_msg.len = msglen;

    if (queue_size(&current_task->send_queue) > 0) {
        Task *send_task = task_at(queue_pop(&current_task->send_queue));
        ipc_recvsend(current_task, send_task);
    } else {
        current_task->status = SENDBLOCKED;
    }
}

void ipc_peek(int tid, int peektid, char *msg, size_t msglen) {
    Task *current_task = task_at(tid);
    Task *peek_task = task_at(peektid);
    if (!peek_task || !ipc_connectable(peek_task)) {
        current_task->tf->r0 = (uint32_t) -1;
        return;
    }
    if (peek_task->status != REPLYBLOCKED) {
        current_task->tf->r0 = (uint32_t) -2;
        return;
    }
    Message dest = {
        .array = msg,
        .len = msglen
    };
    size_t length = msg_copy(&dest, &(peek_task->send_msg));
    current_task->tf->r0 = length;
}

void ipc_reply(int tid, int replytid, char *reply, size_t rplen) {
    Task *current_task = task_at(tid);
    Task *reply_task = task_at(replytid);
    if (!reply_task || !ipc_connectable(reply_task)) {
        current_task->tf->r0 = (uint32_t) -1;
        return;
    }
    if (reply_task->status != REPLYBLOCKED) {
        current_task->tf->r0 = (uint32_t) -2;
        return;
    }
    Message source = {
        .array = reply,
        .len = rplen
    };
    size_t length = msg_copy(&(reply_task->reply_msg), &source);
    reply_task->tf->r0 = length;
    current_task->tf->r0 = length;
    reply_task->status = READY;
}

void ipc_cleanup(int tid) {
    Task *current_task = task_at(tid);
    while (queue_size(&current_task->send_queue)) {
        Task *task = task_at(queue_pop(&current_task->send_queue));
        task->tf->r0 = (uint32_t) -2;
        task->status = READY;
    }
}
