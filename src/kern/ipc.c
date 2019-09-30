#include <kern/ipc.h>
#include <kern/tasks.h>

static int msg_copy(Message *dest, Message *source) {
    unsigned int i = 0;
    for (i = 0; i < source->len && i < dest->len; i++) {
        dest->array[i] = source->array[i];
    }
    return i;
}

static int ipc_connectable(Task *task) {
    return task->status != UNUSED && task->status != ZOMBIE;
}

static void ipc_recvsend(Task *receiver, Task *sender) {
    *(receiver->send_tid) = sender->tid;
    receiver->return_value = msg_copy(&receiver->recv_msg, &sender->send_msg);
    sender->status = REPLYBLOCKED;
}

void ipc_send(int tid, int recvtid, char *msg, int msglen, char *reply, int rplen) {
    Task *current_task = task_at(tid);
    Task *recv_task = task_at(recvtid);
    if (!recv_task || !ipc_connectable(recv_task)) {
        current_task->return_value = -1;
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
        q_push(&recv_task->send_queue, current_task->tid);
        current_task->status = RECVBLOCKED;
    }
}

void ipc_receive(int tid, int *sendtid, char *msg, int msglen) {
    Task *current_task = task_at(tid);

    current_task->send_tid = sendtid;
    current_task->recv_msg.array = msg;
    current_task->recv_msg.len = msglen;

    if (q_size(&current_task->send_queue) > 0) {
        Task *send_task = task_at(q_pop(&current_task->send_queue));
        ipc_recvsend(current_task, send_task);
    } else {
        current_task->status = SENDBLOCKED;
    }
}

void ipc_reply(int tid, int replytid, char *reply, int rplen) {
    Task *current_task = task_at(tid);
    Task *reply_task = task_at(replytid);
    if (!reply_task || !ipc_connectable(reply_task)) {
        current_task->return_value = -1;
        return;
    }
    if (reply_task->status != REPLYBLOCKED) {
        current_task->return_value = -2;
        return;
    }
    Message msg = {
        .array = reply,
        .len = rplen
    };
    int length = msg_copy(&reply_task->reply_msg, &msg);
    reply_task->return_value = length;
    current_task->return_value = length;
    reply_task->status = READY;
}

void ipc_cleanup(int tid) {
    Task *current_task = task_at(tid);
    while (q_size(&current_task->send_queue)) {
        Task *task = task_at(q_pop(&current_task->send_queue));
        task->return_value = -2;
        task->status = READY;
    }
}
