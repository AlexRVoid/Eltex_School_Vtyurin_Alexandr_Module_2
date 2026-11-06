#include "phone_book.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Внутренний поиск для редактирования 
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
                FullName *person, const char *format, ...)
{
    if (!list || !count || !next_id) return -1;
    if (person->lastName[0] == '\0' || person->firstName[0] == '\0') return -1;

    size_t new_count = *count + 1;
    Contact *temp = (Contact *)realloc(*list, new_count * sizeof(Contact));
    if (!temp) {
        return -1;
    }

    *list = temp;
    Contact *new_contact = &((*list)[*count]);
    memset(new_contact, 0, sizeof(Contact));

    new_contact->id = (*next_id)++;
    strncpy(new_contact->name.lastName, person->lastName, MAX_NAME_LEN - 1);
    strncpy(new_contact->name.firstName, person->firstName, MAX_NAME_LEN - 1);

    if (format != NULL) {
        va_list factor;
        va_start(factor, format);

        for (const char *c = format; *c; c++) {
            if (*c != '%') continue;

            switch (*++c) {
                case 's': {
                    //const char *patr = va_arg(factor, const char *);
                    if (person->secondName[0] != '\0') {
                        strncpy(new_contact->name.secondName, person->secondName, MAX_NAME_LEN - 1);
                    }
                    break;
                }
                case 'p': {
                    const char *ph = va_arg(factor, const char *);
                    if (ph && new_contact->phoneCount < MAX_PHONES) {
                        strncpy(new_contact->phones[new_contact->phoneCount], ph, MAX_PHONE_LEN - 1);
                        new_contact->phoneCount++;
                    }
                    break;
                }
                case 'a': {
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
        va_end(factor);
    }

    *count = new_count;
    return new_contact->id;
}

const Contact* contact_find_by_id(const Contact *list, size_t count, int id)
{
    return find_internal((Contact*)list, count, id);
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

    // Сдвиг элементов влево 
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

int contact_update_name(Contact *list, size_t count, int id, const char *format, ...)
{
    Contact *c = find_internal(list, count, id);
    if (!c) return -1;

    if (format != NULL) {
        va_list factor;             
        va_start(factor, format);   
        const FullName *newName = va_arg(factor, const FullName*);
        for (const char *f = format; *f; f++) {//парсинг страны с обозначениями необязательных аргументов
            
            if (*f != '%') {//Пропускаем все символы, не являющиеся спецификаторами формата
                continue;
            }

            switch (*++f) {
                case 'l': { // Фамилия
                    if (newName->lastName[0] != '\0') {
                        strncpy(c->name.lastName, newName->lastName, MAX_NAME_LEN - 1);
                    }
                    break;
                }
                case 'f': { // Имя
                    if (newName->firstName[0] != '\0') {
                        strncpy(c->name.firstName, newName->firstName, MAX_NAME_LEN - 1);
                    }
                    break;
                }
                case 's': { // Отчество
                    if (newName->secondName[0] != '\0') {
                        strncpy(c->name.secondName, newName->secondName, MAX_NAME_LEN - 1);
                    }
                    
                    break;
                }
                default:
                    break;
            }
        }

        va_end(factor); // Завершаем обработку параметров 
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