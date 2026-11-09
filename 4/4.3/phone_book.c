#include "phone_book.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static int g_mod_counter = 0;

/* ---------- Вспомогательные функции дерева ---------- */

int contact_get_height(const Contact *root)
{
    return root ? root->height : 0;
}

static int max_val(int a, int b)
{
    return a > b ? a : b;
}

static void update_height(Contact *n)
{
    if (n) {
        n->height = 1 + max_val(contact_get_height(n->left), contact_get_height(n->right));
    }
}

/* Фактор баланса узла: разница высот левого и правого поддеревьев */
int contact_get_bf(const Contact *root)
{
    return root ? contact_get_height(root->left) - contact_get_height(root->right) : 0;
}

static int contact_compare(const FullName *name_a, int id_a,
                           const FullName *name_b, int id_b)
{
    int cmp = strcmp(name_a->lastName, name_b->lastName);
    if (cmp != 0) return cmp;

    cmp = strcmp(name_a->firstName, name_b->firstName);
    if (cmp != 0) return cmp;

    cmp = strcmp(name_a->secondName, name_b->secondName);
    if (cmp != 0) return cmp;

    return id_a - id_b;
}

static Contact* bst_insert(Contact *root, Contact *new_node)
{
    if (!root) {
        new_node->height = 1;
        return new_node;
    }

    int cmp = contact_compare(&new_node->name, new_node->id,
                              &root->name, root->id);
    if (cmp < 0) {
        root->left = bst_insert(root->left, new_node);
    } else {
        root->right = bst_insert(root->right, new_node);
    }

    update_height(root);
    return root;
}

static Contact* find_internal(Contact *root, int id)
{
    if (!root) return NULL;
    if (root->id == id) return root;

    Contact *found = find_internal(root->left, id);
    if (found) return found;

    return find_internal(root->right, id);
}

static Contact* bst_find_min(Contact *node)
{
    Contact *curr = node;
    while (curr && curr->left != NULL) {
        curr = curr->left;
    }
    return curr;
}

static Contact* bst_delete_node(Contact *root, Contact *target, int *deleted)
{
    if (!root || !target) return NULL;

    int cmp = contact_compare(&target->name, target->id,
                              &root->name, root->id);

    if (cmp < 0) {
        root->left = bst_delete_node(root->left, target, deleted);
    } else if (cmp > 0) {
        root->right = bst_delete_node(root->right, target, deleted);
    } else {
        *deleted = 1;

        if (!root->left) {
            Contact *temp = root->right;
            free(root);
            return temp;
        } else if (!root->right) {
            Contact *temp = root->left;
            free(root);
            return temp;
        }

        Contact *successor = bst_find_min(root->right);

        root->id = successor->id;
        root->name = successor->name;
        memcpy(root->phones, successor->phones, sizeof(root->phones));
        root->phoneCount = successor->phoneCount;
        memcpy(root->addresses, successor->addresses, sizeof(root->addresses));
        root->addressCount = successor->addressCount;

        int dummy = 0;
        root->right = bst_delete_node(root->right, successor, &dummy);
    }

    update_height(root);
    return root;
}

/* ---------- Периодическая балансировка ---------- */

static void collect_inorder(Contact *root, Contact **arr, size_t *index)
{
    if (!root) return;
    collect_inorder(root->left, arr, index);
    arr[(*index)++] = root;
    collect_inorder(root->right, arr, index);
}

static Contact* build_balanced_bst(Contact **arr, int start, int end)
{
    if (start > end) return NULL;

    int mid = start + (end - start) / 2;
    Contact *root = arr[mid];

    root->left = build_balanced_bst(arr, start, mid - 1);
    root->right = build_balanced_bst(arr, mid + 1, end);

    update_height(root);
    return root;
}

int contact_balance(Contact **root, size_t count)
{
    if (!root || !(*root) || count <= 2) return 0;

    Contact **node_array = (Contact **)malloc(count * sizeof(Contact *));
    if (!node_array) return -1;

    size_t index = 0;
    collect_inorder(*root, node_array, &index);

    *root = build_balanced_bst(node_array, 0, (int)count - 1);
    free(node_array);
    return 0;
}

int contact_check_auto_balance(Contact **root, size_t count, int *was_balanced)
{
    if (was_balanced) *was_balanced = 0;
    g_mod_counter++;

    if (g_mod_counter >= AUTO_BALANCE_PERIOD) {
        g_mod_counter = 0;
        if (contact_balance(root, count) == 0) {
            if (was_balanced) *was_balanced = 1;
            return 0;
        }
    }
    return 0;
}

/* ---------- Основные операции ---------- */

int contact_add(Contact **root, size_t *count, int *next_id,
                const FullName *person, const char *format, ...)
{
    if (!root || !count || !next_id || !person) return -1;
    if (person->lastName[0] == '\0' || person->firstName[0] == '\0') return -1;

    Contact *new_node = (Contact *)malloc(sizeof(Contact));
    if (!new_node) return -1;

    memset(new_node, 0, sizeof(Contact));
    new_node->id = (*next_id)++;
    strncpy(new_node->name.lastName, person->lastName, MAX_NAME_LEN - 1);
    strncpy(new_node->name.firstName, person->firstName, MAX_NAME_LEN - 1);

    if (format != NULL) {
        va_list factor;
        va_start(factor, format);

        for (const char *c = format; *c; c++) {
            if (*c != '%') continue;

            switch (*++c) {
                case 's': {
                    if (person->secondName[0] != '\0') {
                        strncpy(new_node->name.secondName, person->secondName, MAX_NAME_LEN - 1);
                    }
                    break;
                }
                case 'p': {
                    const char *ph = va_arg(factor, const char *);
                    if (ph && new_node->phoneCount < MAX_PHONES) {
                        strncpy(new_node->phones[new_node->phoneCount], ph, MAX_PHONE_LEN - 1);
                        new_node->phoneCount++;
                    }
                    break;
                }
                case 'a': {
                    const Address *addr = va_arg(factor, const Address *);
                    if (addr && new_node->addressCount < MAX_ADDRESSES) {
                        new_node->addresses[new_node->addressCount] = *addr;
                        new_node->addressCount++;
                    }
                    break;
                }
                default:
                    break;
            }
        }
        va_end(factor);
    }

    *root = bst_insert(*root, new_node);
    (*count)++;
    return new_node->id;
}

const Contact* contact_find_by_id(const Contact *root, int id)
{
    return find_internal((Contact *)root, id);
}

int contact_delete(Contact **root, size_t *count, int id)
{
    if (!root || !(*root)) return -1;

    Contact *target = find_internal(*root, id);
    if (!target) return -1;

    int deleted = 0;
    *root = bst_delete_node(*root, target, &deleted);

    if (deleted && count && *count > 0) {
        (*count)--;
        return 0;
    }
    return -1;
}

int contact_update_name(Contact **root, int id, const char *format, ...)
{
    if (!root || !(*root) || !format) return -1;

    Contact *target = find_internal(*root, id);
    if (!target) return -1;

    va_list factor;
    va_start(factor, format);
    const FullName *newName = va_arg(factor, const FullName *);

    if (newName != NULL) {
        Contact backup = *target;

        for (const char *f = format; *f; f++) {
            if (*f != '%') continue;

            switch (*++f) {
                case 'l': {
                    if (newName->lastName[0] != '\0') {
                        strncpy(backup.name.lastName, newName->lastName, MAX_NAME_LEN - 1);
                    }
                    break;
                }
                case 'f': {
                    if (newName->firstName[0] != '\0') {
                        strncpy(backup.name.firstName, newName->firstName, MAX_NAME_LEN - 1);
                    }
                    break;
                }
                case 's': {
                    if (newName->secondName[0] != '\0') {
                        strncpy(backup.name.secondName, newName->secondName, MAX_NAME_LEN - 1);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        int deleted = 0;
        *root = bst_delete_node(*root, target, &deleted);

        Contact *reinserted = (Contact *)malloc(sizeof(Contact));
        if (reinserted) {
            *reinserted = backup;
            reinserted->left = NULL;
            reinserted->right = NULL;
            reinserted->height = 1;
            *root = bst_insert(*root, reinserted);
        }
    }

    va_end(factor);
    return 0;
}

int contact_add_phone(Contact *root, int id, const char *phone)
{
    Contact *c = find_internal(root, id);
    if (!c || !phone || c->phoneCount >= MAX_PHONES) return -1;

    strncpy(c->phones[c->phoneCount], phone, MAX_PHONE_LEN - 1);
    c->phoneCount++;
    return 0;
}

int contact_remove_phone(Contact *root, int id, size_t phone_idx)
{
    Contact *c = find_internal(root, id);
    if (!c || phone_idx >= c->phoneCount) return -1;

    for (size_t i = phone_idx; i < c->phoneCount - 1; i++) {
        strncpy(c->phones[i], c->phones[i + 1], MAX_PHONE_LEN - 1);
    }
    c->phoneCount--;
    return 0;
}

int contact_add_address(Contact *root, int id, const Address *addr)
{
    Contact *c = find_internal(root, id);
    if (!c || !addr || c->addressCount >= MAX_ADDRESSES) return -1;

    c->addresses[c->addressCount] = *addr;
    c->addressCount++;
    return 0;
}

int contact_remove_address(Contact *root, int id, size_t addr_idx)
{
    Contact *c = find_internal(root, id);
    if (!c || addr_idx >= c->addressCount) return -1;

    for (size_t i = addr_idx; i < c->addressCount - 1; i++) {
        c->addresses[i] = c->addresses[i + 1];
    }
    c->addressCount--;
    return 0;
}

/* ---------- Канонические обходы дерева ---------- */

static void do_traverse(const Contact *n, TreeOrder order, void (*cb)(const Contact *))
{
    if (!n) return;
    if (order == ORDER_TLR) cb(n);        /* Корень (Top) */
    do_traverse(n->left, order, cb);     /* Левое поддерево */
    if (order == ORDER_LTR) cb(n);        /* Симметричный: по алфавиту */
    do_traverse(n->right, order, cb);    /* Правое поддерево */
    if (order == ORDER_LRT) cb(n);        /* Постфиксный */
}

void contact_traverse(const Contact *root, TreeOrder order, void (*callback)(const Contact *))
{
    if (!root || !callback) return;
    do_traverse(root, order, callback);
}

/* ---------- Вывод дерева с ветками (адаптировано из репозитория) ---------- */

static void print_tree_branches(const Contact *n, const char *prefix, int is_last, int is_root)
{
    if (!n) return;

    if (is_root) {
        printf("%s %s (ID: %d) [h=%d, bf=%d]\n",
               n->name.lastName, n->name.firstName, n->id,
               n->height, contact_get_bf(n));
    } else {
        printf("%s%s%s %s (ID: %d) [h=%d, bf=%d]\n",
               prefix,
               is_last ? "└─ " : "├─ ",
               n->name.lastName, n->name.firstName, n->id,
               n->height, contact_get_bf(n));
    }

    char child_prefix[256];
    if (is_root) {
        snprintf(child_prefix, sizeof(child_prefix), "%s", prefix);
    } else {
        snprintf(child_prefix, sizeof(child_prefix), "%s%s",
                 prefix, is_last ? "   " : "│  ");
    }

    if (n->left || n->right) {
        if (n->left) {
            print_tree_branches(n->left, child_prefix, (n->right == NULL), 0);
        }
        if (n->right) {
            print_tree_branches(n->right, child_prefix, 1, 0);
        }
    }
}

void contact_print_tree(const Contact *root)
{
    if (!root) {
        printf("\n[Дерево пусто]\n");
        return;
    }
    printf("\n=== Структура бинарного дерева поиска ===\n");
    print_tree_branches(root, "", 1, 1);
    printf("=========================================\n");
}

/* ---------- Пресеты для быстрой демонстрации ---------- */

void contact_load_presets(Contact **root, size_t *count, int *next_id)
{
    static const FullName presets[8] = {
        { "Alekseev",  "Alexandr", "Ivanovich" },
        { "Borisov",   "Boris",    "Petrovich" },
        { "Fedorov",   "Fedor",    "Sergeevich" },
        { "Grigoriev", "Grigoriy", "Olegovich" },
        { "Ivanov",    "Ivan",     "Ivanovich" },
        { "Kuznetsov", "Kirill",   "Alexeevich" },
        { "Petrov",    "Petr",     "Vasilievich" },
        { "Sidorov",   "Semen",    "Dmitrievich" }
    };

    for (int i = 0; i < 8; i++) {
        contact_add(root, count, next_id, &presets[i], NULL);
    }
    g_mod_counter = 8;//Изменение значения счётчика изменений
}

void contact_free_all(Contact **root, size_t *count)
{
    if (!root || !(*root)) return;

    contact_free_all(&((*root)->left), NULL);
    contact_free_all(&((*root)->right), NULL);

    free(*root);
    *root = NULL;

    if (count) {
        *count = 0;
        g_mod_counter = 0; /* Сброс счётчика изменений для изоляции тестов */
    }
}