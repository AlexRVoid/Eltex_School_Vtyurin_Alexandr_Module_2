#include "phone_book.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Вспомогательная функция сравнения контактов по алфавиту: Фамилия -> Имя -> Отчество 
static int contact_compare(const FullName *a, const FullName *b)
{
    int cmp = strcmp(a->lastName, b->lastName);
    if (cmp != 0) return cmp;

    cmp = strcmp(a->firstName, b->firstName);
    if (cmp != 0) return cmp;

    return strcmp(a->secondName, b->secondName);
}

// Вставка узла в двухсвязный список с сохранением порядка
static void list_insert_sorted(Contact **head, Contact *new_node)
{
    new_node->prev = NULL;
    new_node->next = NULL;

    // Случай 1: Список пуст  
    if (*head == NULL) {
        *head = new_node;
        return;
    }

    // Случай 2: Новый контакт должен стать первым по алфавиту (меньше головы) 
    if (contact_compare(&new_node->name, &(*head)->name) < 0) {
        new_node->next = *head;
        (*head)->prev = new_node;
        *head = new_node;
        return;
    }

    // Случай 3: Поиск места вставки в середине или конце списка  
    Contact *curr = *head;
    while (curr->next != NULL && contact_compare(&new_node->name, &curr->next->name) >= 0) {
        curr = curr->next;
    }

    // Встраивание new_node после curr ( 
    new_node->next = curr->next;
    new_node->prev = curr;

    if (curr->next != NULL) {
        curr->next->prev = new_node;
    }
    curr->next = new_node;
}

// Извлечение узла из связей списка без освобождения памяти 
static void list_detach_node(Contact **head, Contact *node)
{
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        *head = node->next; // Если удаляли голову списка 
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    }

    node->prev = NULL;
    node->next = NULL;
}

// Поиск контакта по ID перебором списка 
static Contact* find_internal(Contact *head, int id)
{
    for (Contact *curr = head; curr != NULL; curr = curr->next) {
        if (curr->id == id) {
            return curr;
        }
    }
    return NULL;
}

int contact_add(Contact **head, size_t *count, int *next_id,
                const FullName *person, const char *format, ...)
{
    if (!head || !count || !next_id || !person) return -1;
    if (person->lastName[0] == '\0' || person->firstName[0] == '\0') return -1;

    // Выделение памяти под один новый узел списка 
    Contact *new_contact = (Contact *)malloc(sizeof(Contact));
    if (!new_contact) {
        return -1;
    }
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

    // Вставка нового узла в список с сохранением сортировки по алфавиту 
    list_insert_sorted(head, new_contact);

    (*count)++;
    return new_contact->id;
}

const Contact* contact_find_by_id(const Contact *head, int id)
{
    return find_internal((Contact *)head, id);
}

int contact_delete(Contact **head, size_t *count, int id)
{
    if (!head || !(*head)) return -1;

    Contact *target = find_internal(*head, id);
    if (!target) return -1;

    // Исключаем узел из связей списка 
    list_detach_node(head, target);

    // Освобождаем память удаленного узла 
    free(target);

    if (count && *count > 0) {
        (*count)--;
    }
    return 0;
}

int contact_update_name(Contact **head, int id, const char *format, ...)
{
    if (!head || !(*head) || !format) return -1;

    Contact *c = find_internal(*head, id);
    if (!c) return -1;

    va_list factor;
    va_start(factor, format);
    const FullName *newName = va_arg(factor, const FullName *);

    if (newName != NULL) {
        //Временно отсоединяем контакт от списка, чтобы изменение фамилиине нарушило инвариант сортировки связей
        
        list_detach_node(head, c);

        for (const char *f = format; *f; f++) {
            if (*f != '%') continue;

            switch (*++f) {
                case 'l': {
                    if (newName->lastName[0] != '\0') {
                        strncpy(c->name.lastName, newName->lastName, MAX_NAME_LEN - 1);
                    }
                    break;
                }
                case 'f': {
                    if (newName->firstName[0] != '\0') {
                        strncpy(c->name.firstName, newName->firstName, MAX_NAME_LEN - 1);
                    }
                    break;
                }
                case 's': {
                    if (newName->secondName[0] != '\0') {
                        strncpy(c->name.secondName, newName->secondName, MAX_NAME_LEN - 1);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        // Возвращаем узел в список на его новое алфавитное место 
        list_insert_sorted(head, c);
    }

    va_end(factor);
    return 0;
}

int contact_add_phone(Contact *head, int id, const char *phone)
{
    Contact *c = find_internal(head, id);
    if (!c || !phone || c->phoneCount >= MAX_PHONES) return -1;

    strncpy(c->phones[c->phoneCount], phone, MAX_PHONE_LEN - 1);
    c->phoneCount++;
    return 0;
}

int contact_remove_phone(Contact *head, int id, size_t phone_idx)
{
    Contact *c = find_internal(head, id);
    if (!c || phone_idx >= c->phoneCount) return -1;

    for (size_t i = phone_idx; i < c->phoneCount - 1; i++) {
        strncpy(c->phones[i], c->phones[i + 1], MAX_PHONE_LEN - 1);
    }
    c->phoneCount--;
    return 0;
}

int contact_add_address(Contact *head, int id, const Address *addr)
{
    Contact *c = find_internal(head, id);
    if (!c || !addr || c->addressCount >= MAX_ADDRESSES) return -1;

    c->addresses[c->addressCount] = *addr;
    c->addressCount++;
    return 0;
}

int contact_remove_address(Contact *head, int id, size_t addr_idx)
{
    Contact *c = find_internal(head, id);
    if (!c || addr_idx >= c->addressCount) return -1;

    for (size_t i = addr_idx; i < c->addressCount - 1; i++) {
        c->addresses[i] = c->addresses[i + 1];
    }
    c->addressCount--;
    return 0;
}

void contact_free_all(Contact **head, size_t *count)
{
    if (!head || !(*head)) return;

    // Поузловое освобождение памяти списка 
    Contact *curr = *head;
    while (curr != NULL) {
        Contact *next = curr->next;
        free(curr);
        curr = next;
    }

    *head = NULL;
    if (count) {
        *count = 0;
    }
}