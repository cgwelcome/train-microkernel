#ifndef __UTILS_PQueu_H__
#define __UTILS_PQ_H__

#define PQUEUE_SIZE 2048

#include <stdint.h>

typedef struct {
    int id;
    int priority;
} PQueueRecord;

typedef struct {
    PQueueRecord array[PQUEUE_SIZE];
    uint32_t size;
} PQueue;

void pqueue_init(PQueue *pqueue);
int pqueue_pop(PQueue *pqueue);
void pqueue_insert(PQueue *pqueue, int id, int priority);
int pqueue_peek(PQueue *pqueue);
uint32_t pqueue_size(PQueue *pqueue);

#endif
