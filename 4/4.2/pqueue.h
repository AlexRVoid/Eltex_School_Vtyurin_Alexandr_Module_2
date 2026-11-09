#ifndef PQUEUE_H_
#define PQUEUE_H_

#include <stdint.h>
#include <stddef.h>

#define QUEUE_CAPACITY      16
#define NUM_PRIO_LEVELS     8
#define PRIO_LEVEL_STEP     32  // 256 / 8 = 32 приоритета на уровень 

// Результат добавления задачи (запись всегда гарантирована) 
typedef enum PushResult{
    PUSH_SUCCESS = 0,       // Добавлено в свободный слот 
    PUSH_OVERFLOW_EVICTED   // Очередь была полна: вытеснена самая старая задача с худшим приоритетом 
} PushResult;

// Структура задачи 
typedef struct Task{
    int id;                 // Уникальный ID задачи 
    uint8_t prio;           // Точный приоритет (0 - высший, 255 - низший) 
    uint8_t level;          // Уровень приоритета 0..7 (prio / 32) 
    uint32_t seq;           // Счетчик порядка добавления для строгого FIFO 
} Task;

// Узел очереди с двусвязной индексацией 
typedef struct TaskNode {
    Task task;
    struct TaskNode *prev_prio, *next_prio; // Список по приоритету (0 -> 255) 
    struct TaskNode *prev_time, *next_time; // Список по времени прибытия (FIFO) 
} TaskNode;

// Структура очереди 
typedef struct PriorityQueue{
    TaskNode pool[QUEUE_CAPACITY];
    TaskNode *free_list;

    TaskNode *head_prio;    // Наивысший приоритет (ближе к 0) 
    TaskNode *tail_prio;    // Наихудший приоритет (ближе к 255) 

    TaskNode *head_time;    // Самая старая задача в очереди 
    TaskNode *tail_time;    // Самая свежая задача в очереди 

    size_t size;
    uint32_t next_seq;
    int next_id;
} PriorityQueue;

// Названия уровней трафика (QoS) 
const char* pqueue_get_level_name(uint8_t level);

// Инициализация очереди 
void pqueue_init(PriorityQueue *pq);

// Добавление задачи (вытесняет старую с худшим приоритетом при переполнении) 
PushResult pqueue_push(PriorityQueue *pq, uint8_t prio, Task *out_evicted);

// 1) Извлечение первой задачи (наивысший приоритет, при равенстве - самая старая) 
int pqueue_pop_first(PriorityQueue *pq, Task *out_task);

// 2) Извлечение задачи с точным приоритетом 
int pqueue_pop_exact(PriorityQueue *pq, uint8_t prio, Task *out_task);

// 3) Извлечение с приоритетом не ниже заданного 
int pqueue_pop_closest_not_lower(PriorityQueue *pq, uint8_t target_prio, Task *out_task);

// Очистка всей очереди 
void pqueue_clear(PriorityQueue *pq);

#endif /* PQUEUE_H_ */