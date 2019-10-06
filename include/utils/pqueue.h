#ifndef __UTILS_PQueu_H__
#define __UTILS_PQ_H__

#define PQUEUE_SIZE 2048

typedef struct {
    int id;
    int priority;
} PQueueRecord;

typedef struct {
    PQueueRecord array[PQUEUE_SIZE];
    unsigned int size;
} PQueue;

void pqueue_init(PQueue *pqueue);
int pqueue_pop(PQueue *pqueue);
void pqueue_insert(PQueue *pqueue, int id, int priority);
int pqueue_peek(PQueue *pqueue);
unsigned int pqueue_size(PQueue *pqueue);

#endif
