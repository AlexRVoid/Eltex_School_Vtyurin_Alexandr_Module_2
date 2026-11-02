#include "phonebook.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Внутренний поиск для редактирования */
static Contact* find_internal(Contact *list, size_t count, int id)
{
    if (!list) return NULL;
    for (size_t i = 0; i < count; i++) {
        if (list[i].id == id) {
            return &list[i];
        }
    }
    return NULL;
}

int contact_add(Contact **list, size_t *count, int *next_id,
                const char *lastName, const char *firstName,
                const char *format, ...)
{
    if (!list || !count || !next_id || !lastName || !firstName) return -1;
    if (lastName[0] == '\0' || firstName[0] == '\0') return -1;

    /* Выделение памяти под новый контакт в динамическом массиве */
    size_t new_count = *count + 1;
    Contact *temp = (Contact *)realloc(*list, new_count * sizeof(Contact));
    if (!temp) {
        return -1;
    }

    *list = temp;
    Contact *new_contact = &((*list)[*count]);
    memset(new_contact, 0, sizeof(Contact));

    /* Назначение неизменяемого ID */
    new_contact->id = (*next_id)++;

    /* Обязательные поля ФИО */
    strncpy(new_contact->name.lastName, lastName, MAX_NAME_LEN - 1);
    strncpy(new_contact->name.firstName, firstName, MAX_NAME_LEN - 1);

    /* 
     * Разбор необязательных параметров (Лекция 2, слайды 59–60)
     * Используем цикл for по строке формата и switch по следующему символу
     */
    if (format != NULL) {
        va_list factor;             /* Указатель va_list на стек параметров (слайд 56) */
        va_start(factor, format);   /* Связываем factor с последним именованным параметром (слайд 56) */

        for (const char *c = format; *c; c++) {
            /* Пропускаем все символы, не являющиеся спецификаторами формата (слайд 59) */
            if (*c != '%') {
                continue;
            }

            /* Анализируем спецификатор после знака '%' (слайд 60) */
            switch (*++c) {
                case 'o': { /* Отчество: строка const char* */
                    const char *patr = va_arg(factor, const char *);
                    if (patr) {
                        strncpy(new_contact->name.patronymic, patr, MAX_NAME_LEN - 1);
                    }
                    break;
                }
                case 'p': { /* Телефон: строка const char* */
                    const char *ph = va_arg(factor, const char *);
                    if (ph && new_contact->phoneCount < MAX_PHONES) {
                        strncpy(new_contact->phones[new_contact->phoneCount], ph, MAX_PHONE_LEN - 1);
                        new_contact->phoneCount++;
                    }
                    break;
                }
                case 'a': { /* Адрес: указатель на структуру const Address* */
                    const Address *addr = va_arg(factor, const Address *);
                    if (addr && new_contact->addressCount < MAX_ADDRESSES) {
                        new_contact->addresses[new_contact->addressCount] = *addr;
                        new_contact->addressCount++;
                    }
                    break;
                }
                default:
                    break;
            }
        }

        va_end(factor); /* Завершаем обработку параметров (слайд 56) */
    }

    *count = new_count;
    return new_contact->id;
}

const Contact* contact_find_by_id(const Contact *list, size_t count, int id)
{
    if (!list) return NULL;
    for (size_t i = 0; i < count; i++) {
        if (list[i].id == id) {
            return &list[i];
        }
    }
    return NULL;
}

int contact_delete(Contact **list, size_t *count, int id)
{
    if (!list || !(*list) || !count || *count == 0) return -1;

    size_t target_idx = *count;
    for (size_t i = 0; i < *count; i++) {
        if ((*list)[i].id == id) {
            target_idx = i;
            break;
        }
    }

    if (target_idx == *count) return -1;

    for (size_t i = target_idx; i < *count - 1; i++) {
        (*list)[i] = (*list)[i + 1];
    }

    (*count)--;

    if (*count == 0) {
        free(*list);
        *list = NULL;
    } else {
        Contact *temp = (Contact *)realloc(*list, (*count) * sizeof(Contact));
        if (temp) {
            *list = temp;
        }
    }

    return 0;
}

int contact_update_name(Contact *list, size_t count, int id,
                        const char *lastName, const char *firstName, const char *patronymic)
{
    Contact *c = find_internal(list, count, id);
    if (!c) return -1;

    if (lastName && lastName[0] != '\0') {
        strncpy(c->name.lastName, lastName, MAX_NAME_LEN - 1);
    }
    if (firstName && firstName[0] != '\0') {
        strncpy(c->name.firstName, firstName, MAX_NAME_LEN - 1);
    }
    if (patronymic) {
        strncpy(c->name.patronymic, patronymic, MAX_NAME_LEN - 1);
    }
    return 0;
}

int contact_add_phone(Contact *list, size_t count, int id, const char *phone)
{
    Contact *c = find_internal(list, count, id);
    if (!c || !phone || c->phoneCount >= MAX_PHONES) return -1;

    strncpy(c->phones[c->phoneCount], phone, MAX_PHONE_LEN - 1);
    c->phoneCount++;
    return 0;
}

int contact_remove_phone(Contact *list, size_t count, int id, size_t phone_idx)
{
    Contact *c = find_internal(list, count, id);
    if (!c || phone_idx >= c->phoneCount) return -1;

    for (size_t i = phone_idx; i < c->phoneCount - 1; i++) {
        strncpy(c->phones[i], c->phones[i + 1], MAX_PHONE_LEN - 1);
    }
    c->phoneCount--;
    return 0;
}

int contact_add_address(Contact *list, size_t count, int id, const Address *addr)
{
    Contact *c = find_internal(list, count, id);
    if (!c || !addr || c->addressCount >= MAX_ADDRESSES) return -1;

    c->addresses[c->addressCount] = *addr;
    c->addressCount++;
    return 0;
}

int contact_remove_address(Contact *list, size_t count, int id, size_t addr_idx)
{
    Contact *c = find_internal(list, count, id);
    if (!c || addr_idx >= c->addressCount) return -1;

    for (size_t i = addr_idx; i < c->addressCount - 1; i++) {
        c->addresses[i] = c->addresses[i + 1];
    }
    c->addressCount--;
    return 0;
}

void contact_free_all(Contact **list, size_t *count)
{
    if (list && *list) {
        free(*list);
        *list = NULL;
    }
    if (count) {
        *count = 0;
    }
}