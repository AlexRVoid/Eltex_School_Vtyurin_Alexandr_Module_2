#ifndef PHONEBOOK_H_
#define PHONEBOOK_H_

#include <stddef.h>

#define MAX_NAME_LEN    64
#define MAX_PHONE_LEN   32
#define MAX_STR_LEN     128
#define MAX_PHONES      5
#define MAX_ADDRESSES   3

// Автоматическая балансировка срабатывает каждые N изменений 
#define AUTO_BALANCE_PERIOD 3

// Порядки обхода дерева из лекционных материалов 
typedef enum TreeOrder{
    ORDER_LTR, // Симметричный: Левый -> Корень -> Правый (по алфавиту) 
    ORDER_TLR, // Прямой:       Корень -> Левый -> Правый (структурный) 
    ORDER_LRT  // Обратный:     Левый -> Правый -> Корень (постфиксный) 
} TreeOrder;

typedef struct FullName {
    char lastName[MAX_NAME_LEN];   // Обязательное поле 
    char firstName[MAX_NAME_LEN];  // Обязательное поле 
    char secondName[MAX_NAME_LEN]; // Опциональное поле 
} FullName;

typedef struct Address {
    char postalCode[16];
    char country[MAX_NAME_LEN];
    char region[MAX_NAME_LEN];
    char city[MAX_NAME_LEN];
    char street[MAX_STR_LEN];
    char house[16];
    char apartment[16];
} Address;

// Узел бинарного дерева поиска 
typedef struct Contact {
    int id;                                 // Неизменяемый идентификатор 
    FullName name;                          // Вложенная структура ФИО 
    char phones[MAX_PHONES][MAX_PHONE_LEN]; // Массив телефонов 
    size_t phoneCount;
    Address addresses[MAX_ADDRESSES];       // Массив адресов 
    size_t addressCount;

    int height;                             // Высота поддерева для расчета bf 
    struct Contact *left;                   // Левый потомок 
    struct Contact *right;                  // Правый потомок 
} Contact;

// Базовые операции над деревом 
int contact_add(Contact **root, size_t *count, int *next_id,
                const FullName *person, const char *format, ...);

const Contact* contact_find_by_id(const Contact *root, int id);
int contact_delete(Contact **root, size_t *count, int id);
int contact_update_name(Contact **root, int id, const char *format, ...);

int contact_add_phone(Contact *root, int id, const char *phone);
int contact_remove_phone(Contact *root, int id, size_t phone_idx);
int contact_add_address(Contact *root, int id, const Address *addr);
int contact_remove_address(Contact *root, int id, size_t addr_idx);

// Обходы дерева (LTR, TLR, LRT) с функцией обратного вызова (callback) 
void contact_traverse(const Contact *root, TreeOrder order, void (*callback)(const Contact *));

// Расчёт характеристик дерева 
int contact_get_height(const Contact *root);
int contact_get_bf(const Contact *root);

// Балансировка дерева 
int contact_balance(Contact **root, size_t count);
int contact_check_auto_balance(Contact **root, size_t count, int *was_balanced);

// Загрузка готового набора контактов для быстрой защиты 
void contact_load_presets(Contact **root, size_t *count, int *next_id);

// Печать структуры дерева в консоль с префиксами веток 
void contact_print_tree(const Contact *root);

// Очистка всей памяти 
void contact_free_all(Contact **root, size_t *count);

#endif /* PHONEBOOK_H_ */