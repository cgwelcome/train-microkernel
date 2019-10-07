#include <user/tasks.h>
#include <utils/bwio.h>
#include <utils/kassert.h>
#include <utils/pqueue.h>


static void pqueue_single(PQueue *pqueue, int k) {
    pqueue_insert(pqueue, k, k);
}

void pqueue_test_root_task() {
    PQueue pqueue;

    pqueue_init(&pqueue);
    pqueue_single(&pqueue, 8);
    bwprintf(COM2, "%d\r\n", pqueue_peek(&pqueue));
    pqueue_single(&pqueue, 5);
    bwprintf(COM2, "%d\r\n", pqueue_peek(&pqueue));
    pqueue_single(&pqueue, 9);
    bwprintf(COM2, "%d\r\n", pqueue_peek(&pqueue));
    pqueue_single(&pqueue, 1);
    bwprintf(COM2, "%d\r\n", pqueue_peek(&pqueue));
    pqueue_single(&pqueue, 10);
    bwprintf(COM2, "%d\r\n", pqueue_size(&pqueue));
    kassert(5 == pqueue_size(&pqueue));
    kassert(1 == pqueue_peek(&pqueue));
    kassert(1 == pqueue_pop(&pqueue));
    kassert(5 == pqueue_pop(&pqueue));
    kassert(8 == pqueue_pop(&pqueue));
    kassert(9 == pqueue_pop(&pqueue));
    kassert(10 == pqueue_pop(&pqueue));
    Exit();
}
