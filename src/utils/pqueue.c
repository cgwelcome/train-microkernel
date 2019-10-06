#include <utils/pqueue.h>

void pqueue_init(PQueue *pqueue) {
    pqueue->size = 0;
}

static void pqueuerecord_swap(PQueue *pqueue, int i, int j) {
    PQueueRecord record = pqueue->array[i];
    pqueue->array[i] = pqueue->array[j];
    pqueue->array[j] = record;
}

static void pqueue_bubbledown(PQueue *pqueue, int i) {
    unsigned int min = i;
    if (2*i+1 < pqueue->size && pqueue->array[2*i+1].priority < pqueue->array[min].priority) {
        min = 2*i+1;
    }
    if (2*i+2 < pqueue->size && pqueue->array[2*i+2].priority < pqueue->array[min].priority) {
        min = 2*i+2;
    }
    if (min == i) return;
    pqueuerecord_swap(pqueue, i, min);
    pqueue_bubbledown(pqueue, min);
}

int pqueue_pop(PQueue *pqueue) {
    int id = pqueue->array[0].id;
    pqueue->array[0] = pqueue->array[pqueue->size-1];
    pqueue_bubbledown(pqueue, 0);
    pqueue->size--;
    return id;
}

static void pqueue_bubbleup(PQueue *pqueue, int k) {   
    int i = k;
    while (i != 0 && pqueue->array[(i-1)/2].priority > pqueue->array[i].priority) {
        pqueuerecord_swap(pqueue, i, (i-1)/2);
        i = (i-1)/2;
    }
}

void pqueue_insert(PQueue *pqueue, int id, int priority) {
    if (pqueue->size == PQUEUE_SIZE) return;
    
    pqueue->array[pqueue->size].id = id;
    pqueue->array[pqueue->size].priority = priority;
    pqueue_bubbleup(pqueue, pqueue->size);
    pqueue->size++;
}

int pqueue_peek(PQueue *pqueue) {
    if (pqueue->size > 0) {
        return pqueue->array[0].priority;
    }
    return -1;
}

unsigned int pqueue_size(PQueue *pqueue) {
    return pqueue->size;
}
