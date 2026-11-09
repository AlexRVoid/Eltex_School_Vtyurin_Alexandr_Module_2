#include "unity/unity.h"
#include "../phone_book.h"
#include <string.h>
#include <stdlib.h>

static Contact *root = NULL;
static size_t count = 0;
static int next_id = 1;

static char g_traversal_buf[512];

void setUp(void)
{
    root = NULL;
    count = 0;
    next_id = 1;
    g_traversal_buf[0] = '\0';
}

void tearDown(void)
{
    contact_free_all(&root, &count);
    TEST_ASSERT_NULL(root);
    TEST_ASSERT_EQUAL_UINT(0, count);
}

static void record_name_cb(const Contact *c)
{
    strcat(g_traversal_buf, c->name.lastName);
    strcat(g_traversal_buf, ";");
}

/* Рекурсивный валидатор BST-инвариантов и высот */
static size_t assert_bst_structure(const Contact *node, const Contact *min_node, const Contact *max_node)
{
    if (!node) return 0;

    if (min_node) {
        int cmp = strcmp(node->name.lastName, min_node->name.lastName);
        if (cmp == 0) cmp = node->id - min_node->id;
        TEST_ASSERT_TRUE_MESSAGE(cmp > 0, "Нарушен инвариант BST: узел меньше левой границы");
    }

    if (max_node) {
        int cmp = strcmp(node->name.lastName, max_node->name.lastName);
        if (cmp == 0) cmp = node->id - max_node->id;
        TEST_ASSERT_TRUE_MESSAGE(cmp < 0, "Нарушен инвариант BST: узел больше правой границы");
    }

    int hl = contact_get_height(node->left);
    int hr = contact_get_height(node->right);
    int expected_h = 1 + (hl > hr ? hl : hr);

    TEST_ASSERT_EQUAL_INT_MESSAGE(expected_h, node->height, "Неверно рассчитана высота узла");
    TEST_ASSERT_EQUAL_INT_MESSAGE(hl - hr, contact_get_bf(node), "Неверный balance factor узла");

    return 1 + assert_bst_structure(node->left, min_node, node)
             + assert_bst_structure(node->right, node, max_node);
}

/* --- ТЕСТ 1: Пустое дерево --- */
void test_empty_tree_operations(void)
{
    TEST_ASSERT_EQUAL_INT(0, contact_get_height(root));
    TEST_ASSERT_EQUAL_INT(0, contact_get_bf(root));
    TEST_ASSERT_NULL(contact_find_by_id(root, 10));
    TEST_ASSERT_EQUAL_INT(-1, contact_delete(&root, &count, 10));
    TEST_ASSERT_EQUAL_INT(-1, contact_update_name(&root, 10, "%l", NULL));
    TEST_ASSERT_EQUAL_INT(0, contact_balance(&root, count));
}

/* --- ТЕСТ 2: Вставка корня --- */
void test_add_root_node(void)
{
    FullName p = {"Иванов", "Иван", "Иванович"};
    int id = contact_add(&root, &count, &next_id, &p, NULL);

    TEST_ASSERT_EQUAL_INT(1, id);
    TEST_ASSERT_EQUAL_UINT(1, count);
    TEST_ASSERT_NOT_NULL(root);
    
    TEST_ASSERT_EQUAL_STRING("Иванов", root->name.lastName);
    TEST_ASSERT_EQUAL_INT(1, root->height);
    TEST_ASSERT_EQUAL_INT(0, contact_get_bf(root));
    TEST_ASSERT_NULL(root->left);
    TEST_ASSERT_NULL(root->right);

    TEST_ASSERT_EQUAL_UINT(1, assert_bst_structure(root, NULL, NULL));
}

/* --- ТЕСТ 3: Поиск DFS по ID --- */
void test_find_by_id_dfs(void)
{
    FullName p1 = {"Иванов", "И", ""};
    FullName p2 = {"Борисов", "Б", ""};
    FullName p3 = {"Петров", "П", ""};

    int id_i = contact_add(&root, &count, &next_id, &p1, NULL);
    int id_b = contact_add(&root, &count, &next_id, &p2, NULL);
    int id_p = contact_add(&root, &count, &next_id, &p3, NULL);

    TEST_ASSERT_NOT_NULL(contact_find_by_id(root, id_b));
    TEST_ASSERT_NOT_NULL(contact_find_by_id(root, id_p));
    TEST_ASSERT_NOT_NULL(contact_find_by_id(root, id_i));
    TEST_ASSERT_NULL(contact_find_by_id(root, 999));
}

/* --- ТЕСТ 4: Канонические обходы (LTR, TLR, LRT) --- */
void test_canonical_traversals(void)
{
    FullName p1 = {"Иванов", "И", ""};
    FullName p2 = {"Борисов", "Б", ""};
    FullName p3 = {"Петров", "П", ""};

    contact_add(&root, &count, &next_id, &p1, NULL);
    contact_add(&root, &count, &next_id, &p2, NULL);
    contact_add(&root, &count, &next_id, &p3, NULL);

    /* LTR (In-Order) */
    g_traversal_buf[0] = '\0';
    contact_traverse(root, ORDER_LTR, record_name_cb);
    TEST_ASSERT_EQUAL_STRING("Борисов;Иванов;Петров;", g_traversal_buf);

    /* TLR (Pre-Order) */
    g_traversal_buf[0] = '\0';
    contact_traverse(root, ORDER_TLR, record_name_cb);
    TEST_ASSERT_EQUAL_STRING("Иванов;Борисов;Петров;", g_traversal_buf);

    /* LRT (Post-Order) */
    g_traversal_buf[0] = '\0';
    contact_traverse(root, ORDER_LRT, record_name_cb);
    TEST_ASSERT_EQUAL_STRING("Борисов;Петров;Иванов;", g_traversal_buf);
}

/* --- ТЕСТ 5: Удаление листа (0 потомков) --- */
void test_delete_leaf(void)
{
    FullName p1 = {"Иванов", "И", ""};
    FullName p2 = {"Борисов", "Б", ""};

    contact_add(&root, &count, &next_id, &p1, NULL);
    int id_b = contact_add(&root, &count, &next_id, &p2, NULL);

    TEST_ASSERT_EQUAL_INT(0, contact_delete(&root, &count, id_b));
    TEST_ASSERT_EQUAL_UINT(1, count);
    TEST_ASSERT_NULL(root->left);
    assert_bst_structure(root, NULL, NULL);
}

/* --- ТЕСТ 6: Удаление узла с 1 потомком --- */
void test_delete_node_with_one_child(void)
{
    FullName p1 = {"Иванов", "И", ""};
    FullName p2 = {"Борисов", "Б", ""};
    FullName p3 = {"Алексеев", "А", ""};

    contact_add(&root, &count, &next_id, &p1, NULL);
    int id_b = contact_add(&root, &count, &next_id, &p2, NULL);
    contact_add(&root, &count, &next_id, &p3, NULL);

    TEST_ASSERT_EQUAL_INT(0, contact_delete(&root, &count, id_b));
    TEST_ASSERT_EQUAL_UINT(2, count);
    TEST_ASSERT_EQUAL_STRING("Алексеев", root->left->name.lastName);
    assert_bst_structure(root, NULL, NULL);
}

/* --- ТЕСТ 7: Удаление узла с 2 потомками --- */
void test_delete_node_with_two_children(void)
{
    FullName p_root = {"Иванов", "И", ""};
    FullName p_left = {"Борисов", "Б", ""};
    FullName p_right = {"Сидоров", "С", ""};
    FullName p_succ = {"Петров", "П", ""};

    int id_root = contact_add(&root, &count, &next_id, &p_root, NULL);
    contact_add(&root, &count, &next_id, &p_left, NULL);
    contact_add(&root, &count, &next_id, &p_right, NULL);
    contact_add(&root, &count, &next_id, &p_succ, NULL);

    TEST_ASSERT_EQUAL_INT(0, contact_delete(&root, &count, id_root));
    TEST_ASSERT_EQUAL_UINT(3, count);
    TEST_ASSERT_EQUAL_STRING("Петров", root->name.lastName);
    assert_bst_structure(root, NULL, NULL);
}

/* --- ТЕСТ 8: Редактирование ФИО с перестроением топологии --- */
void test_update_name_rebuilds_tree(void)
{
    FullName p1 = {"Иванов", "И", ""};
    FullName p2 = {"Борисов", "Б", ""};

    int id_i = contact_add(&root, &count, &next_id, &p1, NULL);
    contact_add(&root, &count, &next_id, &p2, NULL);

    FullName new_name = {"Алексеев", "И", ""};
    TEST_ASSERT_EQUAL_INT(0, contact_update_name(&root, id_i, "%l", &new_name));

    assert_bst_structure(root, NULL, NULL);

    g_traversal_buf[0] = '\0';
    contact_traverse(root, ORDER_LTR, record_name_cb);
    TEST_ASSERT_EQUAL_STRING("Алексеев;Борисов;", g_traversal_buf);
}

/* --- ТЕСТ 9: Сжатие высоты при балансировке --- */
void test_balance_compresses_height(void)
{
    FullName list[7] = {
        {"Алексеев", "А", ""}, {"Борисов", "Б", ""}, {"Васильев", "В", ""},
        {"Григорьев", "Г", ""}, {"Дмитриев", "Д", ""}, {"Егоров", "Е", ""},
        {"Жуков", "Ж", ""}
    };

    for (int i = 0; i < 7; i++) {
        contact_add(&root, &count, &next_id, &list[i], NULL);
    }

    TEST_ASSERT_EQUAL_INT(7, contact_get_height(root));
    TEST_ASSERT_EQUAL_INT(0, contact_balance(&root, count));

    TEST_ASSERT_EQUAL_INT(3, contact_get_height(root));
    TEST_ASSERT_EQUAL_STRING("Григорьев", root->name.lastName);
    TEST_ASSERT_EQUAL_INT(0, contact_get_bf(root));

    assert_bst_structure(root, NULL, NULL);
}

/* --- ТЕСТ 10: Проверка периода авто-балансировки (AUTO_BALANCE_PERIOD = 3) --- */
void test_auto_balance_periodicity(void)
{
    int was_balanced = 0;

    contact_check_auto_balance(&root, count, &was_balanced);
    TEST_ASSERT_EQUAL_INT(0, was_balanced);

    contact_check_auto_balance(&root, count, &was_balanced);
    TEST_ASSERT_EQUAL_INT(0, was_balanced);

    contact_check_auto_balance(&root, count, &was_balanced);
    TEST_ASSERT_EQUAL_INT(1, was_balanced);
}

/* --- ТЕСТ 11: Пресеты и их автоматическая балансировка --- */
void test_preset_loading_and_auto_balance(void)
{
    contact_load_presets(&root, &count, &next_id);

    TEST_ASSERT_EQUAL_UINT(8, count);
    TEST_ASSERT_EQUAL_INT(9, next_id);

    /* До балансировки цепочка имеет высоту 8 */
    TEST_ASSERT_EQUAL_INT(8, contact_get_height(root));

    /* Проверяем, что g_mod_counter = 8 взвёл триггер авто-балансировки */
    int was_balanced = 0;
    contact_check_auto_balance(&root, count, &was_balanced);
    TEST_ASSERT_EQUAL_INT(1, was_balanced);

    /* После балансировки высота обязана упасть до <= 4 */
    TEST_ASSERT_TRUE(contact_get_height(root) <= 4);
    assert_bst_structure(root, NULL, NULL);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_empty_tree_operations);
    RUN_TEST(test_add_root_node);
    RUN_TEST(test_find_by_id_dfs);
    RUN_TEST(test_canonical_traversals);
    RUN_TEST(test_delete_leaf);
    RUN_TEST(test_delete_node_with_one_child);
    RUN_TEST(test_delete_node_with_two_children);
    RUN_TEST(test_update_name_rebuilds_tree);
    RUN_TEST(test_balance_compresses_height);
    RUN_TEST(test_auto_balance_periodicity);
    RUN_TEST(test_preset_loading_and_auto_balance);
    return UNITY_END();
}