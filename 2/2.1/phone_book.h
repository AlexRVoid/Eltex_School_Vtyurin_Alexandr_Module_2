#ifndef PHONEBOOK_H_
#define PHONEBOOK_H_

#include <stddef.h>

#define MAX_NAME_LEN    64
#define MAX_PHONE_LEN   32
#define MAX_STR_LEN     128
#define MAX_PHONES      5
#define MAX_ADDRESSES   3

/* Вложенная структура: ФИО контакта (Лекция 1, слайд 44) */
typedef struct FullName{
    char lastName[MAX_NAME_LEN];    /* Обязательное поле */
    char firstName[MAX_NAME_LEN];   /* Обязательное поле */
    char patronymic[MAX_NAME_LEN];  /* Опциональное поле */
} FullName;

/* Вложенная структура: Адрес контакта (Лекция 1, слайд 36) */
typedef struct Address{
    char postalCode[16];            /* Почтовый индекс */
    char country[MAX_NAME_LEN];     /* Страна */
    char region[MAX_NAME_LEN];      /* Регион / область */
    char city[MAX_NAME_LEN];        /* Населенный пункт */
    char street[MAX_STR_LEN];       /* Улица */
    char house[16];                 /* Номер дома */
    char apartment[16];             /* Номер квартиры */
} Address;

/* Структура контакта */
typedef struct Contact{
    int id;                                 /* Неизменяемый уникальный идентификатор */
    FullName name;                          /* Вложенная структура ФИО */
    char phones[MAX_PHONES][MAX_PHONE_LEN]; /* Массив номеров телефонов */
    size_t phoneCount;                      /* Текущее число телефонов */
    Address addresses[MAX_ADDRESSES];       /* Массив структур адресов */
    size_t addressCount;                    /* Текущее число адресов */
} Contact;

/* 
 * Добавление контакта с необязательными параметрами по строке формата (Лекция 2, слайды 55, 59).
 * lastName, firstName — обязательные поля.
 * format — строка со спецификаторами: '%o' (отчество), '%p' (телефон), '%a' (адрес).
 */
int contact_add(Contact **list, size_t *count, int *next_id,
                const char *lastName, const char *firstName,
                const char *format, ...);

const Contact* contact_find_by_id(const Contact *list, size_t count, int id);
int contact_delete(Contact **list, size_t *count, int id);

int contact_update_name(Contact *list, size_t count, int id,
                        const char *lastName, const char *firstName, const char *patronymic);

int contact_add_phone(Contact *list, size_t count, int id, const char *phone);
int contact_remove_phone(Contact *list, size_t count, int id, size_t phone_idx);

int contact_add_address(Contact *list, size_t count, int id, const Address *addr);
int contact_remove_address(Contact *list, size_t count, int id, size_t addr_idx);

void contact_free_all(Contact **list, size_t *count);

#endif /* PHONEBOOK_H_ */