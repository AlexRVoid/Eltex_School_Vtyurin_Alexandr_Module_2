#ifndef PQUEUE_H_
#define PQUEUE_H_

#include <stdint.h>
#include <stddef.h>

#define PQUEUE_PRIORITIES 256
#define PQUEUE_DATA_MAX   128

/* Сообщение / элемент очереди */
typedef struct Message{
    uint8_t priority;               /* Приоритет (0..255, где 255 - наивысший) */
    unsigned int seq_id;            /* Порядковый номер генерации для проверки FIFO */
    char data[PQUEUE_DATA_MAX];     /* Текст сообщения / полезная нагрузка */
} Message;

/* Узел связного списка внутри корзины */
typedef struct QueueNode {
    Message msg;
    struct QueueNode *next;
} QueueNode;

/* Однонаправленная FIFO-корзина для одного уровня приоритета */
typedef struct QueueBucket{
    QueueNode *head;
    QueueNode *tail;
    size_t count;
} QueueBucket;

/* Структура многоуровневой приоритетной очереди */
typedef struct PriorityQueue{
    QueueBucket buckets[PQUEUE_PRIORITIES];
    size_t total_count;             /* Общее количество элементов во всех корзинах */
    int max_priority;               /* Текущий наивысший непустой приоритет (-1, если пусто) */
    unsigned int next_seq_id;       /* Глобальный счётчик сообщений */
} PriorityQueue;

/* Инициализация очереди */
void pqueue_init(PriorityQueue *pq);

/* Добавление элемента с заданным приоритетом (сложность O(1)) */
int pqueue_push(PriorityQueue *pq, uint8_t priority, const char *data);

/* 
 * 1) Извлечение элемента, находящегося первым в очереди (наивысший приоритет, 
 * а при равенстве — самый старый по времени добавления)
 * Возврат: 1 - успешно извлечено, 0 - очередь пуста
 */
int pqueue_pop_first(PriorityQueue *pq, Message *out_msg);

/* 
 * 2) Извлечение элемента с указанным точным приоритетом (сложность O(1))
 * Возврат: 1 - успешно извлечено, 0 - элементов с таким приоритетом нет
 */
int pqueue_pop_by_priority(PriorityQueue *pq, uint8_t priority, Message *out_msg);

/* 
 * 3) Извлечение элемента с приоритетом не ниже чем заданный (priority >= min_priority)
 * Извлекает элемент с наивысшим доступным приоритетом из удовлетворяющих условию.
 * Возврат: 1 - успешно извлечено, 0 - подходящих элементов нет
 */
int pqueue_pop_ge_priority(PriorityQueue *pq, uint8_t min_priority, Message *out_msg);

/* Просмотр количества элементов */
size_t pqueue_size(const PriorityQueue *pq);

/* Очистка и освобождение памяти всей очереди */
void pqueue_free_all(PriorityQueue *pq);

#endif /* PQUEUE_H_ */