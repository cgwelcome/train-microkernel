#ifndef __UTILS_PPQUEUE_H__
#define __UTILS_PPQUEUE_H__

#define PPQUEUE_SIZE 2048

#include <stdint.h>

typedef struct {
    uint32_t id;
    uint32_t priority;
    uint32_t index;
} PPQueueRecord;

typedef struct {
    PPQueueRecord *ref[PPQUEUE_SIZE];
    PPQueueRecord array[PPQUEUE_SIZE];
    uint32_t size;
} PPQueue;

void ppqueue_init(PPQueue *ppqueue);
uint32_t ppqueue_size(PPQueue *ppqueue);
uint32_t ppqueue_pop(PPQueue *ppqueue);
void ppqueue_change_priority(PPQueue *ppqueue, uint32_t id, uint32_t priority);
void ppqueue_insert(PPQueue *ppqueue, uint32_t id, uint32_t priority);
uint32_t ppqueue_peek(PPQueue *ppqueue);
uint32_t ppqueue_find_priority(PPQueue *ppqueue, uint32_t id);

#endif /*__UTILS_PPQUEUE_H__*/
