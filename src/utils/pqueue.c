#include <utils/pqueue.h>

void pqueue_init(PQueue *pqueue) {
    pqueue->size = 0;
}

uint32_t pqueue_size(PQueue *pqueue) {
    return pqueue->size;
}

static void pqueue_swap(PQueue *pqueue, uint32_t i, uint32_t j) {
    PQueueRecord record = pqueue->array[i];
    pqueue->array[i] = pqueue->array[j];
    pqueue->array[j] = record;
}

static void pqueue_bubbleup(PQueue *pqueue, uint32_t k) {
    uint32_t i = k;
    while (i != 0 && pqueue->array[(i-1)/2].priority > pqueue->array[i].priority) {
        pqueue_swap(pqueue, i, (i-1)/2);
        i = (i-1)/2;
    }
}

static void pqueue_bubbledown(PQueue *pqueue, uint32_t i) {
    uint32_t min = i;
    if (2*i+1 < pqueue->size && pqueue->array[2*i+1].priority < pqueue->array[min].priority) {
        min = 2*i+1;
    }
    if (2*i+2 < pqueue->size && pqueue->array[2*i+2].priority < pqueue->array[min].priority) {
        min = 2*i+2;
    }
    if (min == i) return;
    pqueue_swap(pqueue, i, min);
    pqueue_bubbledown(pqueue, min);
}

int pqueue_pop(PQueue *pqueue) {
    kassert(pqueue->size > 0);

    int id = pqueue->array[0].id;
    pqueue->array[0] = pqueue->array[pqueue->size-1];
    pqueue_bubbledown(pqueue, 0);
    pqueue->size--;
    return id;
}

void pqueue_insert(PQueue *pqueue, int id, int priority) {
    kassert(pqueue->size < PQUEUE_SIZE);

    pqueue->array[pqueue->size].id = id;
    pqueue->array[pqueue->size].priority = priority;
    pqueue_bubbleup(pqueue, pqueue->size);
    pqueue->size++;
}

int pqueue_peek(PQueue *pqueue) {
    kassert(pqueue->size > 0);
    return pqueue->array[0].priority;
}
