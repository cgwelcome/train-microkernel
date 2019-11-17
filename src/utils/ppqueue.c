#include <utils/assert.h>
#include <utils/ppqueue.h>

void ppqueue_init(PPQueue *ppqueue) {
    ppqueue->size = 0;
}

uint32_t ppqueue_size(PPQueue *ppqueue) {
    return ppqueue->size;
}

uint32_t ppqueue_find_priority(PPQueue *ppqueue, uint32_t id) {
    return ppqueue->array[id].priority;
}

static void ppqueue_write_to_index(PPQueue *ppqueue, uint32_t i, PPQueueRecord *record) {
    ppqueue->ref[i] = record;
    ppqueue->ref[i]->index = i;
}

static void ppqueue_swap(PPQueue *ppqueue, uint32_t i, uint32_t j) {
    PPQueueRecord *record = ppqueue->ref[i];
    ppqueue_write_to_index(ppqueue, i, ppqueue->ref[j]);
    ppqueue_write_to_index(ppqueue, j, record);
}

static void ppqueue_bubbleup(PPQueue *ppqueue, uint32_t k) {
    uint32_t i = k;
    while (i != 0 && ppqueue->ref[(i-1)/2]->priority > ppqueue->ref[i]->priority) {
        ppqueue_swap(ppqueue, i, (i-1)/2);
        i = (i-1)/2;
    }
}

static void ppqueue_bubbledown(PPQueue *ppqueue, uint32_t i) {
    uint32_t min = i;
    if (2*i+1 < ppqueue->size && ppqueue->ref[2*i+1]->priority < ppqueue->ref[min]->priority) {
        min = 2*i+1;
    }
    if (2*i+2 < ppqueue->size && ppqueue->ref[2*i+2]->priority < ppqueue->ref[min]->priority) {
        min = 2*i+2;
    }
    if (min == i) return;
    ppqueue_swap(ppqueue, i, min);
    ppqueue_bubbledown(ppqueue, min);
}

uint32_t ppqueue_pop(PPQueue *ppqueue) {
    assert(ppqueue->size > 0);

    uint32_t id = ppqueue->ref[0]->id;
    ppqueue_write_to_index(ppqueue, 0, ppqueue->ref[ppqueue->size-1]);
    ppqueue_bubbledown(ppqueue, 0);
    ppqueue->size--;
    return id;
}

void ppqueue_change_priority(PPQueue *ppqueue, uint32_t id, uint32_t priority) {
    assert(ppqueue->size < PPQUEUE_SIZE);
    ppqueue->array[id].priority = priority;
    ppqueue_bubbleup(ppqueue, ppqueue->array[id].index);
}

void ppqueue_insert(PPQueue *ppqueue, uint32_t id, uint32_t priority) {
    assert(ppqueue->size < PPQUEUE_SIZE);

    ppqueue->array[id].id = id;
    ppqueue->array[id].priority = priority;
    ppqueue_write_to_index(ppqueue, ppqueue->size, &ppqueue->array[id]);
    ppqueue_bubbleup(ppqueue, ppqueue->size);
    ppqueue->size++;
}

uint32_t ppqueue_peek(PPQueue *ppqueue) {
    assert(ppqueue->size > 0);
    return ppqueue->ref[0]->priority;
}
