#include "queue.h"

void queue_init(Queue *q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

int queue_is_empty(Queue *q) {
    return q->size == 0;
}

int queue_is_full(Queue *q) {
    return q->size == MAX_QUEUE_SIZE;
}

int queue_enqueue(Queue *q, int node_id, int distance) {
    if (queue_is_full(q)) {
        return 0;
    }
    
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->data[q->rear].node_id = node_id;
    q->data[q->rear].distance = distance;
    q->size++;
    
    return 1;
}

QueueElement queue_dequeue(Queue *q) {
    QueueElement element = {-1, -1};
    
    if (queue_is_empty(q)) {
        return element;
    }
    
    element = q->data[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->size--;
    
    return element;
}

int queue_size(Queue *q) {
    return q->size;
}
