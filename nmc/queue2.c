#include <stdlib.h>

#ifndef QUEUE_H_
#define QUEUE_H_

struct queue_node 
{
    struct queue_node* next;
    int data;
};

struct connection_queue 
{
    int size, max_size;
    struct queue_node *head, *tail;
};

void queue_set_size(struct connection_queue* queue, int n)
{
    queue->max_size = n;
    queue->size = 0;
    queue->head = 0;
    queue->tail = 0;
}

int enqueue(struct connection_queue* q, int e)
{
    if (q->size == q->max_size)
    {
        return -1;
    } else {
        struct queue_node* node = malloc(sizeof(struct queue_node));
        if (q->tail == 0)
        {
            q->tail = node;
            q->head = node;
            node->next = 0;
            node->data = e;
        } else {
            q->tail->next = node;
            node->next = 0;
            node->data = e;
            q->tail = node;
        }
        q->size++;
        return 0;
    }
}

int dequeue(struct connection_queue* q)
{
    if (q->size == 0)
    {
        return -1;
    }

    if (q->head == q->tail)
    {
        int data = q->head->data;
        free(q->head);
        q->tail = 0;
        q->head = 0;
        q->size--;
        return data;
    } else {
        int data = q->head->data;
        struct queue_node* temp = q->head->next;
        free(q->head);
        q->head = temp;
        q->size--;
        return data;
    }
}

int queue_size(struct connection_queue* q)
{
    return q->size;
}

int queue_full(struct connection_queue* q)
{
    return q->size == q->max_size;
}   

int queue_empty(struct connection_queue* q)
{
    return q->size == 0;
}
#endif
