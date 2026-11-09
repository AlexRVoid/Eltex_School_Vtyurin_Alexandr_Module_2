#include "user_interface.h"
#include "pqueue.h"
#include <stdio.h>
#include <string.h>

static void read_line(char *dest, size_t max_len)
{
    if (fgets(dest, (int)max_len, stdin) != NULL) {
        size_t len = strlen(dest);
        if (len > 0 && dest[len - 1] == '\n') {
            dest[len - 1] = '\0';
        }
    } else {
        dest[0] = '\0';
    }
}

static void print_task_card(const char *action, const Task *t)
{
    printf("\n==================================================\n");
    printf("[%s] ID: %-3d | Приоритет: %-3u | Seq: #%-3u\n",
           action, t->id, t->prio, t->seq);
    printf("Класс (Уровень %u): %s\n", t->level, pqueue_get_level_name(t->level));
    printf("==================================================\n");
}

static void ui_view_by_priority(const PriorityQueue *pq)
{
    if (pq->size == 0) {
        printf("\nОчередь задач пуста.\n");
        return;
    }

    printf("\n=== ОЧЕРЕДЬ ПО ПРИОРИТЕТУ (0=ВЫСШИЙ -> 255=НИЗШИЙ) ===\n");
    printf("%-4s | %-4s | %-5s | %-5s | %-32s\n", "Pos", "ID", "Prio", "Seq", "Класс трафика (QoS)");
    printf("-------------------------------------------------------------\n");

    int pos = 1;
    for (const TaskNode *n = pq->head_prio; n; n = n->next_prio) {
        printf("#%-3d | %-4d | %-5u | #%-4u | Ур.%u %-27s\n",
               pos++, n->task.id, n->task.prio, n->task.seq,
               n->task.level, pqueue_get_level_name(n->task.level));
    }
    printf("-------------------------------------------------------------\n");
    printf("Заполнено: %zu из %d слотов.\n", pq->size, QUEUE_CAPACITY);
}

static void ui_view_by_arrival(const PriorityQueue *pq)
{
    if (pq->size == 0) {
        printf("\nОчередь задач пуста.\n");
        return;
    }

    printf("\n=== ОЧЕРЕДЬ ПО ВРЕМЕНИ ПРИБЫТИЯ (FIFO ХРОНОЛОГИЯ) ===\n");
    printf("%-5s | %-4s | %-5s | %-32s\n", "Seq", "ID", "Prio", "Класс трафика (QoS)");
    printf("-------------------------------------------------------------\n");

    for (const TaskNode *n = pq->head_time; n; n = n->next_time) {
        printf("#%-4u | %-4d | %-5u | Ур.%u %-27s\n",
               n->task.seq, n->task.id, n->task.prio,
               n->task.level, pqueue_get_level_name(n->task.level));
    }
    printf("-------------------------------------------------------------\n");
}

static void ui_add_task(PriorityQueue *pq)
{
    char buf[64];
    int prio = -1;

    printf("\n--- Добавление задачи в очередь ---\n");
    printf("Введите приоритет (0..255, где 0 - высший): ");
    read_line(buf, sizeof(buf));
    if (sscanf(buf, "%d", &prio) != 1 || prio < 0 || prio > 255) {
        printf("Ошибка: недопустимый приоритет! Введите число от 0 до 255.\n");
        return;
    }

    Task evicted;
    PushResult res = pqueue_push(pq, (uint8_t)prio, &evicted);

    if (res == PUSH_SUCCESS) {
        printf("Задача с приоритетом %d успешно поставлена в очередь (Уровень %d).\n",
               prio, prio / PRIO_LEVEL_STEP);
    } else {
        printf("\n>>> [ЦИКЛИЧЕСКОЕ ВЫТЕСНЕНИЕ]: Буфер полон! <<<\n");
        printf("Задача с приоритетом %d сохранена.\n", prio);
        printf("Вытеснена худшая старая задача: ID %d (Prio %u, Seq #%u, %s)\n",
               evicted.id, evicted.prio, evicted.seq, pqueue_get_level_name(evicted.level));
    }
}

static void ui_pop_first(PriorityQueue *pq)
{
    Task t;
    if (pqueue_pop_first(pq, &t)) {
        print_task_card("ИЗВЛЕЧЕНО: ПЕРВАЯ (ВЫСШИЙ ПРИОРИТЕТ)", &t);
    } else {
        printf("\nОчередь задач пуста!\n");
    }
}

static void ui_pop_exact(PriorityQueue *pq)
{
    char buf[32];
    int prio;

    printf("\nВведите приоритет для извлечения (0..255): ");
    read_line(buf, sizeof(buf));
    if (sscanf(buf, "%d", &prio) != 1 || prio < 0 || prio > 255) {
        printf("Некорректный ввод приоритета!\n");
        return;
    }

    Task t;
    if (pqueue_pop_exact(pq, (uint8_t)prio, &t)) {
        if (t.prio == (uint8_t)prio) {
            print_task_card("ИЗВЛЕЧЕНО: ТОЧНЫЙ ПРИОРИТЕТ", &t);
        } else {
            printf("\n>>> [ФОЛЛБЭК]: Точный приоритет %d не найден! <<<\n", prio);
            printf("Автоматически вызвана выборка не ниже заданного: извлечена ближайшая задача с Prio %u.\n", t.prio);
            print_task_card("ИЗВЛЕЧЕНО: БЛИЖАЙШИЙ НЕ НИЖЕ ЗАДАННОГО", &t);
        }
    } else {
        printf("\nВ очереди нет задач с приоритетом %d или выше (диапазон 0..%d пуст).\n", prio, prio);
    }
}

static void ui_pop_closest(PriorityQueue *pq)
{
    char buf[32];
    int prio;

    printf("\nВведите пороговый приоритет (не ниже чем, 0..255): ");
    read_line(buf, sizeof(buf));
    if (sscanf(buf, "%d", &prio) != 1 || prio < 0 || prio > 255) {
        printf("Некорректный ввод приоритета!\n");
        return;
    }

    Task t;
    if (pqueue_pop_closest_not_lower(pq, (uint8_t)prio, &t)) {
        printf("\n[УСПЕХ] Запрошен порог <= %d. Извлечена ближайшая: Prio %u\n", prio, t.prio);
        print_task_card("ИЗВЛЕЧЕНО: БЛИЖАЙШИЙ НЕ НИЖЕ ЗАДАННОГО", &t);
    } else {
        printf("Нет задач с приоритетом не ниже %d (в диапазоне от 0 до %d).\n", prio, prio);
    }
}

static void ui_run_simulation(PriorityQueue *pq)
{
    printf("\n--- Имитация циклического потока задач (18 задач в очередь CAP=%d) ---\n", QUEUE_CAPACITY);
    pqueue_clear(pq);

    static const uint8_t stream[] = {
        240, 200, 160, 130, 100, 70, 40, 5,
        240, 200, 160, 130, 100, 70, 40, 5,
        10, 255
    };

    for (size_t i = 0; i < sizeof(stream) / sizeof(stream[0]); i++) {
        Task ev;
        PushResult r = pqueue_push(pq, stream[i], &ev);
        if (r == PUSH_SUCCESS) {
            printf("  [ADD]   Prio %-3u -> Сохранено\n", stream[i]);
        } else {
            printf("  [EVICT] Prio %-3u -> Сохранено, вытеснена задача ID %-2d (Prio %-3u, Seq #%u)\n",
                   stream[i], ev.id, ev.prio, ev.seq);
        }
    }
    printf("Имитация завершена. Очередь заполнена актуальными задачами.\n");
}

void run_pqueue_ui(void)
{
    PriorityQueue pq;
    pqueue_init(&pq);

    char menu_buf[16];
    int choice = -1;

    while (1) {
        printf("\n=======================================================\n");
        printf("     ОЧЕРЕДЬ С ПРИОРИТЕТОМ (QoS: 8 УРОВНЕЙ, CAP=%d)     \n", QUEUE_CAPACITY);
        printf("     Задач в очереди: %zu из %d                       \n",
               pq.size, QUEUE_CAPACITY);
        printf("=======================================================\n");
        printf("1. Показать очередь (по приоритету: от 0 к 255)\n");
        printf("2. Показать очередь (по времени прибытия: FIFO)\n");
        printf("3. Добавить задачу (с авто-вытеснением)\n");
        printf("4. Извлечь первую задачу (высший приоритет)\n");
        printf("5. Извлечь задачу с точным приоритетом\n");
        printf("6. Извлечь задачу не ниже приоритета (ближайшую)\n");
        printf("7. Запустить симуляцию циклического переполнения (Preset)\n");
        printf("8. Очистить очередь\n");
        printf("0. Выход\n");
        printf("Выберите пункт меню: ");

        read_line(menu_buf, sizeof(menu_buf));
        if (sscanf(menu_buf, "%d", &choice) != 1) {
            printf("Некорректный ввод! Попробуйте снова.\n");
            continue;
        }

        if (choice == 0) {
            printf("Завершение работы программы...\n");
            break;
        }

        switch (choice) {
            case 1: ui_view_by_priority(&pq); break;
            case 2: ui_view_by_arrival(&pq); break;
            case 3: ui_add_task(&pq); break;
            case 4: ui_pop_first(&pq); break;
            case 5: ui_pop_exact(&pq); break;
            case 6: ui_pop_closest(&pq); break;
            case 7: ui_run_simulation(&pq); break;
            case 8:
                pqueue_clear(&pq);
                printf("Очередь полностью очищена.\n");
                break;
            default:
                printf("Неверный пункт меню!\n");
                break;
        }
    }
}