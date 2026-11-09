#ifndef PHONEBOOK_H_
#define PHONEBOOK_H_

#include <stddef.h>

#define MAX_NAME_LEN    64
#define MAX_PHONE_LEN   32
#define MAX_STR_LEN     128
#define MAX_PHONES      5
#define MAX_ADDRESSES   3

typedef struct FullName {
    char lastName[MAX_NAME_LEN];   // Обязательное поле 
    char firstName[MAX_NAME_LEN];  // Обязательное поле 
    char secondName[MAX_NAME_LEN]; // Опциональное поле 
} FullName;

typedef struct Address {
    char postalCode[16];           // Почтовый индекс 
    char country[MAX_NAME_LEN];    // Страна 
    char region[MAX_NAME_LEN];     // Регион / область 
    char city[MAX_NAME_LEN];       // Населенный пункт 
    char street[MAX_STR_LEN];      // Улица 
    char house[16];                // Номер дома 
    char apartment[16];            // Номер квартиры/офиса 
} Address;

// Узел двухсвязного упорядоченного списка контактов 
 
typedef struct Contact {
    int id;                                 // Неизменяемый идентификатор 
    FullName name;                          // Вложенная структура ФИО 
    char phones[MAX_PHONES][MAX_PHONE_LEN]; // Массив телефонов 
    size_t phoneCount;                      // Текущее число телефонов 
    Address addresses[MAX_ADDRESSES];       // Массив адресов 
    size_t addressCount;                    // Текущее число адресов 

    struct Contact *prev;                   // Указатель на предыдущий контакт 
    struct Contact *next;                   // Указатель на следующий контакт 
} Contact;

/* 
 * Добавление контакта с автоматической сортировкой по алфавиту
 * format: '%s' (отчество), '%p' (телефон), '%a' (адрес).
 */
int contact_add(Contact **head, size_t *count, int *next_id,
                const FullName *person, const char *format, ...);

const Contact* contact_find_by_id(const Contact *head, int id);
int contact_delete(Contact **head, size_t *count, int id);

// Обновление ФИО с автоматической пересортировкой позиции в списке 
int contact_update_name(Contact **head, int id, const char *format, ...);

int contact_add_phone(Contact *head, int id, const char *phone);
int contact_remove_phone(Contact *head, int id, size_t phone_idx);

int contact_add_address(Contact *head, int id, const Address *addr);
int contact_remove_address(Contact *head, int id, size_t addr_idx);

void contact_free_all(Contact **head, size_t *count);

#endif /* PHONEBOOK_H_ */