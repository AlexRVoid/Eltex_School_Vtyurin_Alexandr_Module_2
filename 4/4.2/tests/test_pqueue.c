#include "unity/unity.h"
#include "../pqueue.h"
#include <string.h>

static PriorityQueue pq;

void setUp(void)
{
    pqueue_init(&pq);
}

void tearDown(void)
{
    pqueue_clear(&pq);
    TEST_ASSERT_EQUAL_UINT(0, pq.size);
    TEST_ASSERT_NULL(pq.head_prio);
    TEST_ASSERT_NULL(pq.tail_prio);
    TEST_ASSERT_NULL(pq.head_time);
    TEST_ASSERT_NULL(pq.tail_time);
}

static void assert_pqueue_invariants(const PriorityQueue *q)
{
    if (q->size == 0) {
        TEST_ASSERT_NULL(q->head_prio);
        TEST_ASSERT_NULL(q->tail_prio);
        TEST_ASSERT_NULL(q->head_time);
        TEST_ASSERT_NULL(q->tail_time);
    } else {
        TEST_ASSERT_NOT_NULL(q->head_prio);
        TEST_ASSERT_NOT_NULL(q->tail_prio);
        TEST_ASSERT_NOT_NULL(q->head_time);
        TEST_ASSERT_NOT_NULL(q->tail_time);
        TEST_ASSERT_NULL(q->head_prio->prev_prio);
        TEST_ASSERT_NULL(q->tail_prio->next_prio);
        TEST_ASSERT_NULL(q->head_time->prev_time);
        TEST_ASSERT_NULL(q->tail_time->next_time);
    }

    // 1. Валидация списка по приоритету 
    size_t count_prio = 0;
    const TaskNode *curr_p = q->head_prio;
    const TaskNode *last_p = NULL;

    while (curr_p) {
        count_prio++;
        last_p = curr_p;

        if (curr_p->next_prio) {
            // Проверка симметрии обратной связи 
            TEST_ASSERT_EQUAL_PTR_MESSAGE(curr_p, curr_p->next_prio->prev_prio,
                "Нарушена симметрия prev_prio/next_prio");

            // Проверка упорядоченности приоритетов (0 - высший) 
            TEST_ASSERT_TRUE_MESSAGE(curr_p->task.prio <= curr_p->next_prio->task.prio,
                "Нарушен порядок сортировки по приоритету");

            // Проверка стабильности: при равных prio старый (меньший seq) идет раньше 
            if (curr_p->task.prio == curr_p->next_prio->task.prio) {
                TEST_ASSERT_TRUE_MESSAGE(curr_p->task.seq < curr_p->next_prio->task.seq,
                    "Нарушен FIFO-порядок для одинаковых приоритетов");
            }
        }
        curr_p = curr_p->next_prio;
    }
    TEST_ASSERT_EQUAL_UINT_MESSAGE(q->size, count_prio, "Размер списка приоритетов не равен pq->size");
    TEST_ASSERT_EQUAL_PTR(q->tail_prio, last_p);

    // 2. Валидация списка по времени (FIFO) 
    size_t count_time = 0;
    const TaskNode *curr_t = q->head_time;
    const TaskNode *last_t = NULL;

    while (curr_t) {
        count_time++;
        last_t = curr_t;

        if (curr_t->next_time) {
            TEST_ASSERT_EQUAL_PTR_MESSAGE(curr_t, curr_t->next_time->prev_time,
                "Нарушена симметрия prev_time/next_time");
            TEST_ASSERT_TRUE_MESSAGE(curr_t->task.seq < curr_t->next_time->task.seq,
                "Нарушена монотонность меток времени в списке прибытия");
        }
        curr_t = curr_t->next_time;
    }
    TEST_ASSERT_EQUAL_UINT_MESSAGE(q->size, count_time, "Размер списка времени не равен pq->size");
    TEST_ASSERT_EQUAL_PTR(q->tail_time, last_t);

    // 3. Валидация свободного пула узлов 
    size_t count_free = 0;
    const TaskNode *curr_f = q->free_list;
    while (curr_f) {
        count_free++;
        curr_f = curr_f->next_time;
    }
    TEST_ASSERT_EQUAL_UINT_MESSAGE(QUEUE_CAPACITY, q->size + count_free,
        "Утечка или дублирование узлов в статическом пуле");
}

/* --- ТЕСТ 1: Поведение на пустой очереди --- */
void test_empty_queue_operations(void)
{
    Task t;
    TEST_ASSERT_EQUAL_UINT(0, pq.size);
    TEST_ASSERT_EQUAL_INT(0, pqueue_pop_first(&pq, &t));
    TEST_ASSERT_EQUAL_INT(0, pqueue_pop_exact(&pq, 10, &t));
    TEST_ASSERT_EQUAL_INT(0, pqueue_pop_closest_not_lower(&pq, 50, &t));
    assert_pqueue_invariants(&pq);
}

/* --- ТЕСТ 2: Добавление и извлечение одиночной задачи --- */
void test_single_task_lifecycle(void)
{
    Task evicted;
    PushResult res = pqueue_push(&pq, 42, &evicted);

    TEST_ASSERT_EQUAL_INT(PUSH_SUCCESS, res);
    TEST_ASSERT_EQUAL_UINT(1, pq.size);
    TEST_ASSERT_NOT_NULL(pq.head_prio);
    TEST_ASSERT_EQUAL_PTR(pq.head_prio, pq.tail_prio);
    TEST_ASSERT_EQUAL_PTR(pq.head_time, pq.tail_time);
    TEST_ASSERT_EQUAL_PTR(pq.head_prio, pq.head_time);

    TEST_ASSERT_EQUAL_INT(1, pq.head_prio->task.id);
    TEST_ASSERT_EQUAL_UINT(42, pq.head_prio->task.prio);
    TEST_ASSERT_EQUAL_UINT(1, pq.head_prio->task.level); // 42 / 32 = 1 
    TEST_ASSERT_EQUAL_UINT(1, pq.head_prio->task.seq);

    assert_pqueue_invariants(&pq);

    Task out;
    TEST_ASSERT_EQUAL_INT(1, pqueue_pop_first(&pq, &out));
    TEST_ASSERT_EQUAL_INT(1, out.id);
    TEST_ASSERT_EQUAL_UINT(42, out.prio);
    TEST_ASSERT_EQUAL_UINT(0, pq.size);

    assert_pqueue_invariants(&pq);
}

/* --- ТЕСТ 3: Маппинг 8 уровней сетевого приоритета (QoS) --- */
void test_qos_levels_mapping(void)
{
    // Вставляем граничные значения каждого из 8 уровней (шаг 32) 
    uint8_t sample_prios[8] = { 0, 32, 64, 96, 128, 160, 192, 224 };

    for (int i = 0; i < 8; i++) {
        pqueue_push(&pq, sample_prios[i], NULL);
    }
    TEST_ASSERT_EQUAL_UINT(8, pq.size);
    assert_pqueue_invariants(&pq);

    for (int expected_level = 0; expected_level < 8; expected_level++) {
        Task t;
        TEST_ASSERT_EQUAL_INT(1, pqueue_pop_first(&pq, &t));
        TEST_ASSERT_EQUAL_UINT(expected_level, t.level);
        TEST_ASSERT_NOT_NULL(pqueue_get_level_name(t.level));
    }
}

/* --- ТЕСТ 4: Стабильность очереди: FIFO-порядок для одинаковых приоритетов --- */
void test_fifo_tie_breaking(void)
{
    // Добавляем 4 задачи с одинаковым приоритетом 50 
    pqueue_push(&pq, 50, NULL); // ID 1, seq 1 
    pqueue_push(&pq, 50, NULL); // ID 2, seq 2 
    pqueue_push(&pq, 50, NULL); // ID 3, seq 3 
    pqueue_push(&pq, 50, NULL); // ID 4, seq 4 

    assert_pqueue_invariants(&pq);

    Task t;
    // Извлечение обязано пройти строго в хронологическом порядке прибытия 
    TEST_ASSERT_EQUAL_INT(1, pqueue_pop_first(&pq, &t));
    TEST_ASSERT_EQUAL_INT(1, t.id);

    TEST_ASSERT_EQUAL_INT(1, pqueue_pop_first(&pq, &t));
    TEST_ASSERT_EQUAL_INT(2, t.id);

    TEST_ASSERT_EQUAL_INT(1, pqueue_pop_first(&pq, &t));
    TEST_ASSERT_EQUAL_INT(3, t.id);

    TEST_ASSERT_EQUAL_INT(1, pqueue_pop_first(&pq, &t));
    TEST_ASSERT_EQUAL_INT(4, t.id);

    TEST_ASSERT_EQUAL_UINT(0, pq.size);
    assert_pqueue_invariants(&pq);
}

/* --- ТЕСТ 5: Сортировка перемешанного потока (0 = наивысший) --- */
void test_interleaved_priority_sorting(void)
{
    pqueue_push(&pq, 200, NULL);
    pqueue_push(&pq, 10,  NULL);
    pqueue_push(&pq, 150, NULL);
    pqueue_push(&pq, 0,   NULL);
    pqueue_push(&pq, 75,  NULL);

    assert_pqueue_invariants(&pq);

    uint8_t expected_order[5] = { 0, 10, 75, 150, 200 };
    for (int i = 0; i < 5; i++) {
        Task t;
        TEST_ASSERT_EQUAL_INT(1, pqueue_pop_first(&pq, &t));
        TEST_ASSERT_EQUAL_UINT(expected_order[i], t.prio);
    }
}

/* --- ТЕСТ 6: Заполнение пула до максимума (CAPACITY = 16) --- */
void test_full_capacity_fill(void)
{
    for (int i = 0; i < QUEUE_CAPACITY; i++) {
        Task ev;
        PushResult res = pqueue_push(&pq, (uint8_t)(i * 10), &ev);
        TEST_ASSERT_EQUAL_INT(PUSH_SUCCESS, res);
    }

    TEST_ASSERT_EQUAL_UINT(QUEUE_CAPACITY, pq.size);
    TEST_ASSERT_NULL(pq.free_list);
    assert_pqueue_invariants(&pq);
}

/* --- ТЕСТ 7: Циклическое вытеснение: самая старая задача среди худших --- */
void test_eviction_oldest_among_worst(void)
{
    /* 
     * Заполняем 16 слотов:
     * - 2 задачи с наихудшим приоритетом 220: ID 1 (seq 1) и ID 2 (seq 2).
     * - 14 задач с высоким приоритетом 30 (ID 3..16).
     */
    pqueue_push(&pq, 220, NULL); // ID 1 
    pqueue_push(&pq, 220, NULL); // ID 2 

    for (int i = 0; i < 14; i++) {
        pqueue_push(&pq, 30, NULL);
    }
    TEST_ASSERT_EQUAL_UINT(QUEUE_CAPACITY, pq.size);
    assert_pqueue_invariants(&pq);

    // 17-я задача: должна вытесниться ID 1 (приоритет 220, seq 1 — самая ранняя из худших) 
    Task evicted;
    PushResult res = pqueue_push(&pq, 10, &evicted);
    TEST_ASSERT_EQUAL_INT(PUSH_OVERFLOW_EVICTED, res);
    TEST_ASSERT_EQUAL_INT(1, evicted.id);
    TEST_ASSERT_EQUAL_UINT(220, evicted.prio);
    TEST_ASSERT_EQUAL_UINT(QUEUE_CAPACITY, pq.size);
    assert_pqueue_invariants(&pq);

    // 18-я задача: теперь худшей осталась задача ID 2 (приоритет 220, seq 2) 
    res = pqueue_push(&pq, 15, &evicted);
    TEST_ASSERT_EQUAL_INT(PUSH_OVERFLOW_EVICTED, res);
    TEST_ASSERT_EQUAL_INT(2, evicted.id);
    TEST_ASSERT_EQUAL_UINT(220, evicted.prio);
    TEST_ASSERT_EQUAL_UINT(QUEUE_CAPACITY, pq.size);
    assert_pqueue_invariants(&pq);
}

/* --- ТЕСТ 8: Безусловная запись низкоприоритетной задачи при переполнении --- */
void test_overflow_unconditional_worst_incoming(void)
{
    // Заполняем все 16 слотов задачами с высоким приоритетом 20 
    for (int i = 0; i < QUEUE_CAPACITY; i++) {
        pqueue_push(&pq, 20, NULL);
    }

    // Приходит задача с худшим приоритетом 255: она обязана записаться, вытеснив ID 1 
    Task evicted;
    PushResult res = pqueue_push(&pq, 255, &evicted);
    TEST_ASSERT_EQUAL_INT(PUSH_OVERFLOW_EVICTED, res);
    TEST_ASSERT_EQUAL_INT(1, evicted.id);
    TEST_ASSERT_EQUAL_UINT(20, evicted.prio);
    TEST_ASSERT_EQUAL_UINT(QUEUE_CAPACITY, pq.size);
    assert_pqueue_invariants(&pq);

    // Проверяем, что задача 255 встала в хвост списка приоритетов 
    TEST_ASSERT_EQUAL_UINT(255, pq.tail_prio->task.prio);
}

/* --- ТЕСТ 9: Извлечение с точным приоритетом (прямое попадание) --- */
void test_pop_exact_hit(void)
{
    pqueue_push(&pq, 10, NULL);
    pqueue_push(&pq, 45, NULL);
    pqueue_push(&pq, 80, NULL);

    Task t;
    TEST_ASSERT_EQUAL_INT(1, pqueue_pop_exact(&pq, 45, &t));
    TEST_ASSERT_EQUAL_UINT(45, t.prio);
    TEST_ASSERT_EQUAL_UINT(2, pq.size);
    assert_pqueue_invariants(&pq);
}

/* --- ТЕСТ 10: Извлечение по точному приоритету с фоллбэком на ближайший --- */
void test_pop_exact_fallback_to_closest(void)
{
    // В очереди лежат приоритеты: 10, 18, 50 
    pqueue_push(&pq, 10, NULL);
    pqueue_push(&pq, 18, NULL);
    pqueue_push(&pq, 50, NULL);

    Task t;
    TEST_ASSERT_EQUAL_INT(1, pqueue_pop_exact(&pq, 20, &t));
    TEST_ASSERT_EQUAL_UINT(18, t.prio);
    TEST_ASSERT_EQUAL_UINT(2, pq.size);
    assert_pqueue_invariants(&pq);

    // Запрашиваем точный 15 (его нет, кандидаты <= 15: только 10) 
    TEST_ASSERT_EQUAL_INT(1, pqueue_pop_exact(&pq, 15, &t));
    TEST_ASSERT_EQUAL_UINT(10, t.prio);
    TEST_ASSERT_EQUAL_UINT(1, pq.size);
    assert_pqueue_invariants(&pq);

    // Запрашиваем 5: точного нет, и нет задач <= 5 -> возвращает 0 
    TEST_ASSERT_EQUAL_INT(0, pqueue_pop_exact(&pq, 5, &t));
    TEST_ASSERT_EQUAL_UINT(1, pq.size);
    assert_pqueue_invariants(&pq);
}

/* --- ТЕСТ 11: Прямой поиск задачи с приоритетом не ниже заданного --- */
void test_pop_closest_not_lower_boundaries(void)
{
    pqueue_push(&pq, 5,   NULL);
    pqueue_push(&pq, 30,  NULL);
    pqueue_push(&pq, 100, NULL);

    Task t;
    // Граничный случай: точное совпадение с границей порога 
    TEST_ASSERT_EQUAL_INT(1, pqueue_pop_closest_not_lower(&pq, 30, &t));
    TEST_ASSERT_EQUAL_UINT(30, t.prio);

    // Запрос порога 255: извлекается наихудший среди доступных (100) 
    TEST_ASSERT_EQUAL_INT(1, pqueue_pop_closest_not_lower(&pq, 255, &t));
    TEST_ASSERT_EQUAL_UINT(100, t.prio);

    // Запрос порога 4: задач <= 4 нет (осталась только 5) 
    TEST_ASSERT_EQUAL_INT(0, pqueue_pop_closest_not_lower(&pq, 4, &t));
    TEST_ASSERT_EQUAL_UINT(1, pq.size);
    assert_pqueue_invariants(&pq);
}

/* --- ТЕСТ 12: Длительная циклическая нагрузка (Churn Test) --- */
void test_continuous_cyclic_churn(void)
{
    // 100 циклов добавления и периодического извлечения 
    for (int i = 0; i < 100; i++) {
        uint8_t p = (uint8_t)((i * 37) % 256);
        pqueue_push(&pq, p, NULL);

        if (i % 3 == 0) {
            Task t;
            pqueue_pop_first(&pq, &t);
        }
        assert_pqueue_invariants(&pq);
    }
    TEST_ASSERT_TRUE(pq.size <= QUEUE_CAPACITY);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_empty_queue_operations);
    RUN_TEST(test_single_task_lifecycle);
    RUN_TEST(test_qos_levels_mapping);
    RUN_TEST(test_fifo_tie_breaking);
    RUN_TEST(test_interleaved_priority_sorting);
    RUN_TEST(test_full_capacity_fill);
    RUN_TEST(test_eviction_oldest_among_worst);
    RUN_TEST(test_overflow_unconditional_worst_incoming);
    RUN_TEST(test_pop_exact_hit);
    RUN_TEST(test_pop_exact_fallback_to_closest);
    RUN_TEST(test_pop_closest_not_lower_boundaries);
    RUN_TEST(test_continuous_cyclic_churn);
    return UNITY_END();
}