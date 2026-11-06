#include "user_interface.h"
#include "phone_book.h"
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

static void print_contact_card(const Contact *c)
{
    if (!c) return;

    printf("\n==================================================\n");
    printf("ID контакта: %d\n", c->id);
    printf("ФИО: %s %s %s\n", c->name.lastName, c->name.firstName, c->name.secondName);

    printf("Номера телефонов (%zu):\n", c->phoneCount);
    if (c->phoneCount == 0) {
        printf("  [нет номеров]\n");
    } else {
        for (size_t i = 0; i < c->phoneCount; i++) {
            printf("  [%zu] %s\n", i + 1, c->phones[i]);
        }
    }

    printf("Адреса (%zu):\n", c->addressCount);
    if (c->addressCount == 0) {
        printf("  [нет адресов]\n");
    } else {
        for (size_t i = 0; i < c->addressCount; i++) {
            const Address *a = &c->addresses[i];
            printf("  [%zu] %s, %s, %s, г. %s, ул. %s, д. %s, кв. %s\n",
                   i + 1, a->postalCode, a->country, a->region, 
                   a->city, a->street, a->house, a->apartment);
        }
    }
    printf("==================================================\n");
}

static void ui_add_contact(Contact **head, size_t *count, int *next_id)
{
    FullName person;
    char phone[MAX_PHONE_LEN];
    char choice[16];
    Address addr;
    int has_address = 0;

    printf("\n--- Создание нового контакта ---\n");

    printf("Фамилия (обязательно): ");
    read_line(person.lastName, MAX_NAME_LEN);
    if (person.lastName[0] == '\0') {
        printf("Ошибка: фамилия не может быть пустой!\n");
        return;
    }

    printf("Имя (обязательно): ");
    read_line(person.firstName, MAX_NAME_LEN);
    if (person.firstName[0] == '\0') {
        printf("Ошибка: имя не может быть пустым!\n");
        return;
    }

    printf("Отчество (Enter, если нет): ");
    read_line(person.secondName, MAX_NAME_LEN);

    printf("Добавить номер телефона? (1 - да, 0 - нет): ");
    read_line(choice, sizeof(choice));
    if (choice[0] == '1') {
        printf("Номер телефона: ");
        read_line(phone, sizeof(phone));
    } else {
        phone[0] = '\0';
    }

    printf("Добавить адрес? (1 - да, 0 - нет): ");
    read_line(choice, sizeof(choice));
    if (choice[0] == '1') {
        memset(&addr, 0, sizeof(Address));
        printf("Почтовый индекс[]: "); read_line(addr.postalCode, sizeof(addr.postalCode));
        printf("Страна[]:          "); read_line(addr.country, sizeof(addr.country));
        printf("Регион/область[]:  "); read_line(addr.region, sizeof(addr.region));
        printf("Город[]:           "); read_line(addr.city, sizeof(addr.city));
        printf("Улица[]:           "); read_line(addr.street, sizeof(addr.street));
        printf("Дом[]:             "); read_line(addr.house, sizeof(addr.house));
        printf("Квартира/офис[]:   "); read_line(addr.apartment, sizeof(addr.apartment));
        has_address = 1;
    }

    int new_id = -1;
    if (person.secondName[0] != '\0' && phone[0] != '\0' && has_address) {
        new_id = contact_add(head, count, next_id, &person, "%s%p%a", phone, &addr);
    } else if (person.secondName[0] != '\0' && phone[0] != '\0') {
        new_id = contact_add(head, count, next_id, &person, "%s%p", phone);
    } else if (person.secondName[0] != '\0' && has_address) {
        new_id = contact_add(head, count, next_id, &person, "%s%a", &addr);
    } else if (phone[0] != '\0' && has_address) {
        new_id = contact_add(head, count, next_id, &person, "%p%a", phone, &addr);
    } else if (person.secondName[0] != '\0') {
        new_id = contact_add(head, count, next_id, &person, "%s", NULL);
    } else if (phone[0] != '\0') {
        new_id = contact_add(head, count, next_id, &person, "%p", phone);
    } else if (has_address) {
        new_id = contact_add(head, count, next_id, &person, "%a", &addr);
    } else {
        new_id = contact_add(head, count, next_id, &person, NULL);
    }

    if (new_id > 0) {
        printf("Контакт успешно добавлен! Присвоен ID: %d (вставлен по алфавиту)\n", new_id);
    } else {
        printf("Ошибка выделения памяти при создании контакта.\n");
    }
}

/* Вывод всех контактов проходом по списку через указатель next */
static void ui_view_all(const Contact *head, size_t count)
{
    if (!head || count == 0) {
        printf("\nТелефонная книга пуста.\n");
        return;
    }

    printf("\n=== Упорядоченный список контактов (Всего: %zu) ===\n", count);
    for (const Contact *curr = head; curr != NULL; curr = curr->next) {
        print_contact_card(curr);
    }
}

static void ui_find_by_id(const Contact *head)
{
    char buf[32];
    int id;

    printf("\nВведите ID контакта: ");
    read_line(buf, sizeof(buf));
    if (sscanf(buf, "%d", &id) != 1) {
        printf("Некорректный ввод ID!\n");
        return;
    }

    const Contact *c = contact_find_by_id(head, id);
    if (c) {
        print_contact_card(c);
    } else {
        printf("Контакт с ID %d не найден.\n", id);
    }
}

static void ui_delete(Contact **head, size_t *count)
{
    char buf[32];
    int id;

    printf("\nВведите ID контакта для удаления: ");
    read_line(buf, sizeof(buf));
    if (sscanf(buf, "%d", &id) != 1) {
        printf("Некорректный ввод ID!\n");
        return;
    }

    if (contact_delete(head, count, id) == 0) {
        printf("Контакт с ID %d успешно удален.\n", id);
    } else {
        printf("Контакт с ID %d не найден.\n", id);
    }
}

static void ui_edit(Contact **head)
{
    char buf[32];
    int id;

    printf("\nВведите ID контакта для редактирования: ");
    read_line(buf, sizeof(buf));
    if (sscanf(buf, "%d", &id) != 1) {
        printf("Некорректный ввод ID!\n");
        return;
    }

    const Contact *c = contact_find_by_id(*head, id);
    if (!c) {
        printf("Контакт с ID %d не найден.\n", id);
        return;
    }

    printf("\nРедактирование контакта ID %d (ID изменить нельзя!)\n", id);
    printf("1. Изменить ФИО (с авто-пересортировкой)\n");
    printf("2. Добавить телефон\n");
    printf("3. Удалить телефон\n");
    printf("4. Добавить адрес\n");
    printf("5. Удалить адрес\n");
    printf("0. Отмена\n");
    printf("Выберите действие: ");

    read_line(buf, sizeof(buf));
    int choice = -1;
    sscanf(buf, "%d", &choice);

    switch (choice) {
        case 1: {
            FullName upConName;

            printf("Новая фамилия (Enter, чтобы оставить '%s'): ", c->name.lastName);
            read_line(upConName.lastName, MAX_NAME_LEN);
            printf("Новое имя (Enter, чтобы оставить '%s'): ", c->name.firstName);
            read_line(upConName.firstName, MAX_NAME_LEN);
            printf("Новое отчество (Enter, чтобы оставить '%s'): ", c->name.secondName);
            read_line(upConName.secondName, MAX_NAME_LEN);

            char format[8] = "";
            if (upConName.lastName[0] != '\0')   strcat(format, "%l");
            if (upConName.firstName[0] != '\0')  strcat(format, "%f");
            if (upConName.secondName[0] != '\0') strcat(format, "%s");

            if (format[0] != '\0') {
                contact_update_name(head, id, format, &upConName);
                printf("ФИО обновлено. Контакт перемещен на новое место по алфавиту.\n");
            } else {
                printf("Изменения не внесены (все поля оставлены пустыми).\n");
            }
            break;
        }
        case 2: {
            char phone[MAX_PHONE_LEN];
            printf("Введите новый номер телефона: ");
            read_line(phone, sizeof(phone));
            if (contact_add_phone(*head, id, phone) == 0) {
                printf("Номер добавлен.\n");
            } else {
                printf("Ошибка: превышен лимит телефонов (%d).\n", MAX_PHONES);
            }
            break;
        }
        case 3: {
            if (c->phoneCount == 0) {
                printf("У контакта нет сохраненных телефонов.\n");
                break;
            }
            printf("Номера телефонов (%zu):\n", c->phoneCount);
            for (size_t i = 0; i < c->phoneCount; i++) {
                printf("  [%zu] %s\n", i + 1, c->phones[i]);
            }
            printf("Введите порядковый номер телефона (1..%zu): ", c->phoneCount);
            read_line(buf, sizeof(buf));
            int idx;
            if (sscanf(buf, "%d", &idx) == 1 && idx >= 1 && (size_t)idx <= c->phoneCount) {
                contact_remove_phone(*head, id, (size_t)(idx - 1));
                printf("Номер удален.\n");
            } else {
                printf("Неверный номер записи.\n");
            }
            break;
        }
        case 4: {
            Address a;
            memset(&a, 0, sizeof(Address));
            printf("Индекс:   "); read_line(a.postalCode, sizeof(a.postalCode));
            printf("Страна:   "); read_line(a.country, sizeof(a.country));
            printf("Регион:   "); read_line(a.region, sizeof(a.region));
            printf("Город:    "); read_line(a.city, sizeof(a.city));
            printf("Улица:    "); read_line(a.street, sizeof(a.street));
            printf("Дом:      "); read_line(a.house, sizeof(a.house));
            printf("Квартира: "); read_line(a.apartment, sizeof(a.apartment));

            if (contact_add_address(*head, id, &a) == 0) {
                printf("Адрес добавлен.\n");
            } else {
                printf("Ошибка: превышен лимит адресов (%d).\n", MAX_ADDRESSES);
            }
            break;
        }
        case 5: {
            if (c->addressCount == 0) {
                printf("У контакта нет сохраненных адресов.\n");
                break;
            }
            printf("Адреса (%zu):\n", c->addressCount);
            for (size_t i = 0; i < c->addressCount; i++) {
                const Address *a = &c->addresses[i];
                printf("  [%zu] %s, %s, %s, г. %s, ул. %s, д. %s, кв. %s\n",
                       i + 1, a->postalCode, a->country, a->region, 
                       a->city, a->street, a->house, a->apartment);
            }
            printf("Введите порядковый номер адреса (1..%zu): ", c->addressCount);
            read_line(buf, sizeof(buf));
            int idx;
            if (sscanf(buf, "%d", &idx) == 1 && idx >= 1 && (size_t)idx <= c->addressCount) {
                contact_remove_address(*head, id, (size_t)(idx - 1));
                printf("Адрес удален.\n");
            } else {
                printf("Неверный номер записи.\n");
            }
            break;
        }
        default:
            break;
    }
}

void run_phonebook_ui(void)
{
    Contact *head = NULL; /* Голова двухсвязного списка */
    size_t count = 0;
    int next_id = 1;

    char menu_buf[16];
    int choice = -1;

    while (1) {
        printf("\n============================================\n");
        printf("      ТЕЛЕФОННАЯ КНИГА  \n");
        printf("============================================\n");
        printf("1. Показать все контакты\n");
        printf("2. Найти контакт по ID\n");
        printf("3. Добавить контакт\n");
        printf("4. Редактировать контакт\n");
        printf("5. Удалить контакт\n");
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
            case 1: ui_view_all(head, count); break;
            case 2: ui_find_by_id(head); break;
            case 3: ui_add_contact(&head, &count, &next_id); break;
            case 4: ui_edit(&head); break;
            case 5: ui_delete(&head, &count); break;
            default:
                printf("Неверный пункт меню!\n");
                break;
        }
    }

    contact_free_all(&head, &count);
}