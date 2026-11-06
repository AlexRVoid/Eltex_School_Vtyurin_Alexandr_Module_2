#include "pqueue.h"
#include <stdlib.h>
#include <string.h>

void pqueue_init(PriorityQueue *pq)
{
    if (!pq) return;

    for (int i = 0; i < PQUEUE_PRIORITIES; i++) {
        pq->buckets[i].head = NULL;
        pq->buckets[i].tail = NULL;
        pq->buckets[i].count = 0;
    }
    pq->total_count = 0;
    pq->max_priority = -1;
    pq->next_seq_id = 1;
}

/* Вспомогательная функция: актуализация индекса максимального приоритета */
static void update_max_priority_after_pop(PriorityQueue *pq)
{
    if (pq->total_count == 0) {
        pq->max_priority = -1;
        return;
    }

    /* Сканируем вниз от текущего max_priority до первого непустого бакета */
    while (pq->max_priority >= 0 && pq->buckets[pq->max_priority].count == 0) {
        pq->max_priority--;
    }
}

/* Вспомогательная функция: извлечение элемента из головы конкретной корзины */
static void bucket_pop_head(PriorityQueue *pq, uint8_t priority, Message *out_msg)
{
    QueueBucket *bucket = &pq->buckets[priority];
    QueueNode *node = bucket->head;

    if (out_msg) {
        *out_msg = node->msg;
    }

    bucket->head = node->next;
    if (bucket->head == NULL) {
        bucket->tail = NULL;
    }
    bucket->count--;
    pq->total_count--;

    free(node);

    /* Если опустел текущий наивысший бакет, обновляем max_priority */
    if (priority == (uint8_t)pq->max_priority && bucket->count == 0) {
        update_max_priority_after_pop(pq);
    }
}

int pqueue_push(PriorityQueue *pq, uint8_t priority, const char *data)
{
    if (!pq || !data) return -1;

    QueueNode *new_node = (QueueNode *)malloc(sizeof(QueueNode));
    if (!new_node) return -1;

    new_node->msg.priority = priority;
    new_node->msg.seq_id = pq->next_seq_id++;
    strncpy(new_node->msg.data, data, PQUEUE_DATA_MAX - 1);
    new_node->msg.data[PQUEUE_DATA_MAX - 1] = '\0';
    new_node->next = NULL;

    QueueBucket *bucket = &pq->buckets[priority];

    /* Добавление в конец FIFO-очереди соответствующего приоритета за O(1) */
    if (bucket->tail == NULL) {
        bucket->head = new_node;
        bucket->tail = new_node;
    } else {
        bucket->tail->next = new_node;
        bucket->tail = new_node;
    }

    bucket->count++;
    pq->total_count++;

    /* Подтягиваем max_priority, если пришел элемент более высокого приоритета */
    if ((int)priority > pq->max_priority) {
        pq->max_priority = (int)priority;
    }

    return 0;
}

int pqueue_pop_first(PriorityQueue *pq, Message *out_msg)
{
    if (!pq || pq->total_count == 0 || pq->max_priority < 0) {
        return 0;
    }

    bucket_pop_head(pq, (uint8_t)pq->max_priority, out_msg);
    return 1;
}

int pqueue_pop_by_priority(PriorityQueue *pq, uint8_t priority, Message *out_msg)
{
    if (!pq || pq->buckets[priority].count == 0) {
        return 0;
    }

    bucket_pop_head(pq, priority, out_msg);
    return 1;
}

int pqueue_pop_ge_priority(PriorityQueue *pq, uint8_t min_priority, Message *out_msg)
{
    if (!pq || pq->total_count == 0 || pq->max_priority < 0) {
        return 0;
    }

    /* 
     * Так как max_priority хранит глобальный максимум в очереди,
     * если он меньше min_priority, то элементов >= min_priority гарантированно нет.
     */
    if (pq->max_priority < (int)min_priority) {
        return 0;
    }

    /* Извлекаем наивысший доступный элемент, удовлетворяющий условию */
    bucket_pop_head(pq, (uint8_t)pq->max_priority, out_msg);
    return 1;
}

size_t pqueue_size(const PriorityQueue *pq)
{
    return pq ? pq->total_count : 0;
}

void pqueue_free_all(PriorityQueue *pq)
{
    if (!pq) return;

    for (int i = 0; i < PQUEUE_PRIORITIES; i++) {
        QueueNode *curr = pq->buckets[i].head;
        while (curr != NULL) {
            QueueNode *tmp = curr->next;
            free(curr);
            curr = tmp;
        }
        pq->buckets[i].head = NULL;
        pq->buckets[i].tail = NULL;
        pq->buckets[i].count = 0;
    }
    pq->total_count = 0;
    pq->max_priority = -1;
}