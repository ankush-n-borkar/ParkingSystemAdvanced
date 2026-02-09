#ifndef QUEUE_H
#define QUEUE_H

#define MAX_QUEUE_SIZE 1000

/* Queue element for BFS */
typedef struct {
    int node_id;
    int distance;
} QueueElement;

/* Queue structure */
typedef struct {
    QueueElement data[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
} Queue;

/* Queue operations */
void queue_init(Queue *q);
int queue_is_empty(Queue *q);
int queue_is_full(Queue *q);
int queue_enqueue(Queue *q, int node_id, int distance);
QueueElement queue_dequeue(Queue *q);
int queue_size(Queue *q);

#endif
