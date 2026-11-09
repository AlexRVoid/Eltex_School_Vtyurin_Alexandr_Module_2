#include "unity/unity.h"
#include "../phone_book.h"
#include <string.h>

static Contact *head = NULL;
static size_t count = 0;
static int next_id = 1;

void setUp(void)//Вызывается перед каждым отдельным тестом
{
    head = NULL;
    count = 0;
    next_id = 1;
}

void tearDown(void) //Вызывается после каждого отдельного теста
{
    contact_free_all(&head, &count);
    TEST_ASSERT_NULL(head);
    TEST_ASSERT_EQUAL_UINT(0, count);
}

/* Вспомогательная функция проверки инвариантов двухсвязного списка */
static void assert_list_invariants(const Contact *h, size_t expected_count)
{
    if (expected_count == 0) {
        TEST_ASSERT_NULL(h);
        return;
    }

    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_NULL(h->prev); /* У головы нет предыдущего */

    size_t actual_count = 0;
    const Contact *curr = h;
    const Contact *last = NULL;

    while (curr != NULL) {
        actual_count++;
        last = curr;

        /* Проверка взаимной связности: если есть следующий, он должен указывать назад */
        if (curr->next != NULL) {
            TEST_ASSERT_EQUAL_PTR(curr, curr->next->prev);
            /* Проверка сохранения алфавитного порядка */
            TEST_ASSERT_TRUE(strcmp(curr->name.lastName, curr->next->name.lastName) <= 0);
        }
        curr = curr->next;
    }

    TEST_ASSERT_EQUAL_UINT(expected_count, actual_count);
    if (last != NULL) {
        TEST_ASSERT_NULL(last->next); /* У хвоста нет следующего */
    }
}

/* --- ТЕСТ 1: Поведение на пустом списке --- */
void test_empty_list_operations(void)
{
    TEST_ASSERT_NULL(contact_find_by_id(head, 1));
    TEST_ASSERT_EQUAL_INT(-1, contact_delete(&head, &count, 1));
    TEST_ASSERT_EQUAL_INT(-1, contact_update_name(&head, 1, "%l", NULL));
}

/* --- ТЕСТ 2: Вставка одного контакта --- */
void test_add_single_contact(void)
{
    FullName p = {"Иванов", "Иван", "Иванович"};
    int id = contact_add(&head, &count, &next_id, &p, NULL);

    TEST_ASSERT_EQUAL_INT(1, id);
    TEST_ASSERT_EQUAL_UINT(1, count);
    TEST_ASSERT_NOT_NULL(head);
    TEST_ASSERT_EQUAL_STRING("Иванов", head->name.lastName);
    TEST_ASSERT_NULL(head->prev);
    TEST_ASSERT_NULL(head->next);

    assert_list_invariants(head, 1);
}

/* --- ТЕСТ 3: Автоматическая сортировка при вставке (Голова, Хвост, Середина) --- */
void test_insert_sorted_order(void)
{
    FullName p_mid = {"Борисов", "Борис", ""};
    FullName p_head = {"Алексеев", "Алексей", ""};
    FullName p_tail = {"Яковлев", "Яков", ""};
    FullName p_mid2 = {"Васильев", "Василий", ""};

    contact_add(&head, &count, &next_id, &p_mid, NULL);   /* 1. Борисов */
    contact_add(&head, &count, &next_id, &p_head, NULL);  /* 2. Алексеев (должен стать головой) */
    contact_add(&head, &count, &next_id, &p_tail, NULL);  /* 3. Яковлев (должен уйти в хвост) */
    contact_add(&head, &count, &next_id, &p_mid2, NULL);  /* 4. Васильев (между Борисовым и Яковлевым) */

    TEST_ASSERT_EQUAL_UINT(4, count);

    /* Проверяем строгий порядок обхода вперед */
    Contact *c = head;
    TEST_ASSERT_EQUAL_STRING("Алексеев", c->name.lastName);
    c = c->next;
    TEST_ASSERT_EQUAL_STRING("Борисов", c->name.lastName);
    c = c->next;
    TEST_ASSERT_EQUAL_STRING("Васильев", c->name.lastName);
    c = c->next;
    TEST_ASSERT_EQUAL_STRING("Яковлев", c->name.lastName);
    TEST_ASSERT_NULL(c->next);

    assert_list_invariants(head, 4);
}

/* --- ТЕСТ 4: Поиск по уникальному ID --- */
void test_find_by_id(void)
{
    FullName p1 = {"Алексеев", "А", ""};
    FullName p2 = {"Борисов", "Б", ""};

    int id1 = contact_add(&head, &count, &next_id, &p1, NULL);
    int id2 = contact_add(&head, &count, &next_id, &p2, NULL);

    const Contact *found1 = contact_find_by_id(head, id1);
    const Contact *found2 = contact_find_by_id(head, id2);
    const Contact *not_found = contact_find_by_id(head, 999);

    TEST_ASSERT_NOT_NULL(found1);
    TEST_ASSERT_EQUAL_STRING("Алексеев", found1->name.lastName);

    TEST_ASSERT_NOT_NULL(found2);
    TEST_ASSERT_EQUAL_STRING("Борисов", found2->name.lastName);

    TEST_ASSERT_NULL(not_found);
}

/* --- ТЕСТ 5: Удаление элементов (Голова, Хвост, Середина) --- */
void test_delete_nodes(void)
{
    FullName p1 = {"Алексеев", "А", ""};
    FullName p2 = {"Борисов", "Б", ""};
    FullName p3 = {"Васильев", "В", ""};

    int id_a = contact_add(&head, &count, &next_id, &p1, NULL);
    int id_b = contact_add(&head, &count, &next_id, &p2, NULL);
    int id_v = contact_add(&head, &count, &next_id, &p3, NULL);

    /* 1. Удаление из середины (Борисов) */
    TEST_ASSERT_EQUAL_INT(0, contact_delete(&head, &count, id_b));
    TEST_ASSERT_EQUAL_UINT(2, count);
    assert_list_invariants(head, 2);
    TEST_ASSERT_EQUAL_STRING("Васильев", head->next->name.lastName);

    /* 2. Удаление головы (Алексеев) */
    TEST_ASSERT_EQUAL_INT(0, contact_delete(&head, &count, id_a));
    TEST_ASSERT_EQUAL_UINT(1, count);
    TEST_ASSERT_EQUAL_STRING("Васильев", head->name.lastName);
    assert_list_invariants(head, 1);

    /* 3. Удаление единственного оставшегося элемента */
    TEST_ASSERT_EQUAL_INT(0, contact_delete(&head, &count, id_v));
    TEST_ASSERT_EQUAL_UINT(0, count);
    TEST_ASSERT_NULL(head);
}

/* --- ТЕСТ 6: Перемещение контакта при изменении ФИО --- */
void test_update_name_reorders_list(void)
{
    FullName p1 = {"Борисов", "Борис", ""};
    FullName p2 = {"Васильев", "Василий", ""};

    int id_b = contact_add(&head, &count, &next_id, &p1, NULL);
    contact_add(&head, &count, &next_id, &p2, NULL);

    /* Меняем Борисова на Яковлева -> контакт обязан уехать в конец */
    FullName new_name = {"Яковлев", "Борис", ""};
    TEST_ASSERT_EQUAL_INT(0, contact_update_name(&head, id_b, "%l", &new_name));

    TEST_ASSERT_EQUAL_STRING("Васильев", head->name.lastName);
    TEST_ASSERT_EQUAL_STRING("Яковлев", head->next->name.lastName);
    assert_list_invariants(head, 2);

    /* Меняем Яковлева на Алексеева -> контакт обязан стать новой головой */
    FullName new_name2 = {"Алексеев", "Борис", ""};
    TEST_ASSERT_EQUAL_INT(0, contact_update_name(&head, id_b, "%l", &new_name2));

    TEST_ASSERT_EQUAL_STRING("Алексеев", head->name.lastName);
    TEST_ASSERT_EQUAL_STRING("Васильев", head->next->name.lastName);
    assert_list_invariants(head, 2);
}

/* --- ТЕСТ 7: Вариативные аргументы (телефоны и адреса) --- */
void test_varargs_and_limits(void)
{
    FullName p = {"Иванов", "Иван", "Иванович"};
    Address addr = {"123456", "Россия", "НСО", "Новосибирск", "Ленина", "1", "10"};

    int id = contact_add(&head, &count, &next_id, &p, "%s%p%a", "+79991112233", &addr);
    Contact *c = (Contact *)contact_find_by_id(head, id);

    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQUAL_UINT(1, c->phoneCount);
    TEST_ASSERT_EQUAL_STRING("+79991112233", c->phones[0]);
    TEST_ASSERT_EQUAL_UINT(1, c->addressCount);
    TEST_ASSERT_EQUAL_STRING("Новосибирск", c->addresses[0].city);

    /* Проверка лимита телефонов (максимум MAX_PHONES = 5) */
    TEST_ASSERT_EQUAL_INT(0, contact_add_phone(head, id, "+79991112234"));
    TEST_ASSERT_EQUAL_INT(0, contact_add_phone(head, id, "+79991112235"));
    TEST_ASSERT_EQUAL_INT(0, contact_add_phone(head, id, "+79991112236"));
    TEST_ASSERT_EQUAL_INT(0, contact_add_phone(head, id, "+79991112237"));
    TEST_ASSERT_EQUAL_INT(-1, contact_add_phone(head, id, "+79991112238")); /* Превышение лимита */
    TEST_ASSERT_EQUAL_UINT(5, c->phoneCount);

    /* Удаление телефона со сдвигом */
    TEST_ASSERT_EQUAL_INT(0, contact_remove_phone(head, id, 0));
    TEST_ASSERT_EQUAL_UINT(4, c->phoneCount);
    TEST_ASSERT_EQUAL_STRING("+79991112234", c->phones[0]);

    
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_empty_list_operations);
    RUN_TEST(test_add_single_contact);
    RUN_TEST(test_insert_sorted_order);
    RUN_TEST(test_find_by_id);
    RUN_TEST(test_delete_nodes);
    RUN_TEST(test_update_name_reorders_list);
    RUN_TEST(test_varargs_and_limits);
    return UNITY_END();
}