#include <utils/assert.h>
#include <utils/queue.h>

void queue_init(Queue *queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}

uint32_t queue_size(Queue *queue) {
    return queue->size;
}

int queue_pop(Queue *queue) {
    assert(queue->size > 0);

    int data = queue->array[queue->head];
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    return data;
}

void queue_push(Queue *queue, int data) {
    assert(queue->size < QUEUE_SIZE);

    queue->array[queue->tail] = data;
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->size++;
}

int queue_peek(Queue *queue) {
    assert(queue->size > 0);
    return queue->array[queue->head];
}
