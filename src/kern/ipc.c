#include <kern/ipc.h>
#include <kern/tasks.h>

static void msg_copy(Message *dest, Message *source) {
    for (unsigned int i = 0; i < source->len && i < dest->len; i++) {
        dest->array[i] = source->array[i];
    }
}

static int ipc_connectable(Task *task) {
    if (task->status == UNUSED || task->status == ZOMBIE) {
        return 0;
    }
    return 1;
}

static void ipc_recvsend(Task *receiver, Task *sender) {
    *(receiver->send_tid) = sender->tid;
    msg_copy(&receiver->recv_msg, &sender->send_msg);
    sender->status = REPLYBLOCKED;
}

int ipc_send(int tid, int recvtid, char *msg, int msglen, char *reply, int rplen) {
    Task *current_task = task_at(tid);
    Task *recv_task = task_at(recvtid);
    if (!recv_task || !ipc_connectable(recv_task)) {
        return -1;
    }
    current_task->send_msg.array = msg;
    current_task->send_msg.len = msglen;
    current_task->reply_msg.array = reply;
    current_task->reply_msg.len = rplen;

    if (recv_task->status == SENDBLOCKED) {
        ipc_recvsend(recv_task, current_task);
        recv_task->status = READY;
    }
    else {
        q_push(&recv_task->send_queue, current_task->tid);
        current_task->status = RECVBLOCKED;
    }
    return 0;
}

int ipc_receive(int tid, int *sendtid, char *msg, int msglen) {
    Task *current_task = task_at(tid);

    current_task->recv_msg.array = msg;
    current_task->recv_msg.len = msglen;
    current_task->send_tid = sendtid;

    if (q_size(&current_task->send_queue)) {
        Task *send_task = task_at(q_pop(&current_task->send_queue));
        ipc_recvsend(current_task, send_task);
    }
    else {
        current_task->status = SENDBLOCKED;
    }
    return 0;
}

int ipc_reply(int tid, int replytid, char *reply, int rplen) {
    Task *reply_task = task_at(replytid);
    if (!reply_task || !ipc_connectable(reply_task)) {
        return -1;
    }
    Message msg = {
        .array = reply,
        .len = rplen
    };
    msg_copy(&reply_task->reply_msg, &msg);
    reply_task->status = READY;
    return 0;
}

void ipc_cleanup(int tid) {
    Task *current_task = task_at(tid);
    while (q_size(&current_task->send_queue)) {
        Task *task = task_at(q_pop(&current_task->send_queue));
        task->return_value = -2;
        task->status = READY;
    }
}
