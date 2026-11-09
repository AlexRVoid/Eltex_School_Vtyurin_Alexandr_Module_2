#include "pqueue.h"
#include <string.h>

static const char *LEVEL_NAMES[NUM_PRIO_LEVELS] = {
    "Network Control [0..31]",
    "Voice / Real-Time [32..63]",
    "Video Streaming [64..95]",
    "Controlled Load [96..127]",
    "High Priority [128..159]",
    "Normal Traffic [160..191]",
    "Low Priority [192..223]",
    "Background / Best Effort [224..255]"
};

const char* pqueue_get_level_name(uint8_t level)
{
    if (level >= NUM_PRIO_LEVELS) return "Unknown";
    return LEVEL_NAMES[level];
}

void pqueue_init(PriorityQueue *pq)
{
    if (!pq) return;
    memset(pq, 0, sizeof(*pq));

    for (size_t i = 0; i < QUEUE_CAPACITY - 1; i++) {
        pq->pool[i].next_time = &pq->pool[i + 1];
    }
    pq->pool[QUEUE_CAPACITY - 1].next_time = NULL;
    pq->free_list = &pq->pool[0];

    pq->next_id = 1;
    pq->next_seq = 1;
}

static void unlink_node(PriorityQueue *pq, TaskNode *node)
{
    // Исключение из цепочки времени (FIFO) 
    if (node->prev_time) node->prev_time->next_time = node->next_time;
    else                 pq->head_time = node->next_time;

    if (node->next_time) node->next_time->prev_time = node->prev_time;
    else                 pq->tail_time = node->prev_time;

    // Исключение из цепочки приоритетов 
    if (node->prev_prio) node->prev_prio->next_prio = node->next_prio;
    else                 pq->head_prio = node->next_prio;

    if (node->next_prio) node->next_prio->prev_prio = node->prev_prio;
    else                 pq->tail_prio = node->prev_prio;

    node->next_prio = node->prev_prio = NULL;
    node->next_time = node->prev_time = NULL;
    pq->size--;
}

static void insert_node(PriorityQueue *pq, TaskNode *node)
{
    // Вставка в хвост хронологического списка времени 
    node->prev_time = pq->tail_time;
    node->next_time = NULL;
    if (pq->tail_time) pq->tail_time->next_time = node;
    else               pq->head_time = node;
    pq->tail_time = node;

    // Вставка в список приоритетов (0 -> 255), при равенстве - FIFO (по seq) 
    TaskNode *curr = pq->head_prio;
    while (curr && (curr->task.prio < node->task.prio ||
                   (curr->task.prio == node->task.prio && curr->task.seq <= node->task.seq))) {
        curr = curr->next_prio;
    }

    if (curr) {
        node->next_prio = curr;
        node->prev_prio = curr->prev_prio;
        if (curr->prev_prio) curr->prev_prio->next_prio = node;
        else                 pq->head_prio = node;
        curr->prev_prio = node;
    } else {
        node->prev_prio = pq->tail_prio;
        node->next_prio = NULL;
        if (pq->tail_prio) pq->tail_prio->next_prio = node;
        else               pq->head_prio = node;
        pq->tail_prio = node;
    }

    pq->size++;
}

static TaskNode* find_eviction_victim(PriorityQueue *pq)
{
    if (!pq->tail_prio) return NULL;

    uint8_t worst_prio = pq->tail_prio->task.prio;
    TaskNode *victim = pq->tail_prio;

    while (victim->prev_prio && victim->prev_prio->task.prio == worst_prio) {
        victim = victim->prev_prio;
    }

    return victim;
}

PushResult pqueue_push(PriorityQueue *pq, uint8_t prio, Task *out_evicted)
{
    if (!pq) return PUSH_SUCCESS;

    int was_evicted = 0;

    // Переполнение: циклическое вытеснение худшей старой задачи 
    if (pq->size >= QUEUE_CAPACITY || !pq->free_list) {
        TaskNode *victim = find_eviction_victim(pq);
        if (victim) {
            if (out_evicted) {
                *out_evicted = victim->task;
            }
            unlink_node(pq, victim);
            victim->next_time = pq->free_list;
            pq->free_list = victim;
            was_evicted = 1;
        }
    }

    TaskNode *new_node = pq->free_list;
    pq->free_list = pq->free_list->next_time;

    new_node->task.id = pq->next_id++;
    new_node->task.prio = prio;
    new_node->task.level = prio / PRIO_LEVEL_STEP;
    new_node->task.seq = pq->next_seq++;

    insert_node(pq, new_node);

    return was_evicted ? PUSH_OVERFLOW_EVICTED : PUSH_SUCCESS;
}

int pqueue_pop_first(PriorityQueue *pq, Task *out_task)
{
    if (!pq || !pq->head_prio) return 0;

    TaskNode *target = pq->head_prio;
    if (out_task) *out_task = target->task;

    unlink_node(pq, target);
    target->next_time = pq->free_list;
    pq->free_list = target;

    return 1;
}

int pqueue_pop_exact(PriorityQueue *pq, uint8_t prio, Task *out_task)
{
    if (!pq || !pq->head_prio) return 0;

    TaskNode *curr = pq->head_prio;
    while (curr && curr->task.prio < prio) {
        curr = curr->next_prio;
    }

    if (curr && curr->task.prio == prio) {
        if (out_task) *out_task = curr->task;
        unlink_node(pq, curr);
        curr->next_time = pq->free_list;
        pq->free_list = curr;
        return 1;
    }

    return pqueue_pop_closest_not_lower(pq, prio, out_task);
}

int pqueue_pop_closest_not_lower(PriorityQueue *pq, uint8_t target_prio, Task *out_task)
{
    if (!pq || !pq->head_prio) return 0;

    TaskNode *curr = pq->head_prio;
    TaskNode *best = NULL;

    while (curr && curr->task.prio <= target_prio) {
        if (!best || curr->task.prio > best->task.prio) {
            best = curr;
        }
        curr = curr->next_prio;
    }

    if (best) {
        if (out_task) *out_task = best->task;
        unlink_node(pq, best);
        best->next_time = pq->free_list;
        pq->free_list = best;
        return 1;
    }

    return 0;
}

void pqueue_clear(PriorityQueue *pq)
{
    if (!pq) return;
    pqueue_init(pq);
}