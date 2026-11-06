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
    printf("ID: %d | ФИО: %s %s %s [h=%d, bf=%d]\n",
           c->id, c->name.lastName, c->name.firstName, c->name.secondName,
           c->height, contact_get_bf(c));

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

static void handle_auto_balance_notify(Contact **root, size_t count)
{
    int was_balanced = 0;
    int h_before = contact_get_height(*root);
    contact_check_auto_balance(root, count, &was_balanced);

    if (was_balanced) {
        int h_after = contact_get_height(*root);
        printf("\n>>> [ПЕРИОДИЧЕСКАЯ БАЛАНСИРОВКА]: Дерево перестроено! <<<\n");
        printf("    Высота дерева: %d -> %d (узлов: %zu)\n", h_before, h_after, count);
    }
}

static void ui_add_contact(Contact **root, size_t *count, int *next_id)
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
        printf("Почтовый индекс: "); read_line(addr.postalCode, sizeof(addr.postalCode));
        printf("Страна:          "); read_line(addr.country, sizeof(addr.country));
        printf("Регион/область:  "); read_line(addr.region, sizeof(addr.region));
        printf("Город:           "); read_line(addr.city, sizeof(addr.city));
        printf("Улица:           "); read_line(addr.street, sizeof(addr.street));
        printf("Дом:             "); read_line(addr.house, sizeof(addr.house));
        printf("Квартира/офис:   "); read_line(addr.apartment, sizeof(addr.apartment));
        has_address = 1;
    }

    int new_id = -1;
    if (person.secondName[0] != '\0' && phone[0] != '\0' && has_address) {
        new_id = contact_add(root, count, next_id, &person, "%s%p%a", phone, &addr);
    } else if (person.secondName[0] != '\0' && phone[0] != '\0') {
        new_id = contact_add(root, count, next_id, &person, "%s%p", phone);
    } else if (person.secondName[0] != '\0' && has_address) {
        new_id = contact_add(root, count, next_id, &person, "%s%a", &addr);
    } else if (phone[0] != '\0' && has_address) {
        new_id = contact_add(root, count, next_id, &person, "%p%a", phone, &addr);
    } else if (person.secondName[0] != '\0') {
        new_id = contact_add(root, count, next_id, &person, "%s", NULL);
    } else if (phone[0] != '\0') {
        new_id = contact_add(root, count, next_id, &person, "%p", phone);
    } else if (has_address) {
        new_id = contact_add(root, count, next_id, &person, "%a", &addr);
    } else {
        new_id = contact_add(root, count, next_id, &person, NULL);
    }

    if (new_id > 0) {
        printf("Контакт сохранен. Присвоен ID: %d\n", new_id);
        handle_auto_balance_notify(root, *count);
    } else {
        printf("Ошибка выделения памяти при создании контакта.\n");
    }
}

static void ui_view_traversal(const Contact *root, size_t count, TreeOrder order, const char *title)
{
    if (!root || count == 0) {
        printf("\nТелефонная книга пуста.\n");
        return;
    }

    printf("\n=== %s (Всего: %zu) ===\n", title, count);
    contact_traverse(root, order, print_contact_card);
}

static void ui_find_by_id(const Contact *root)
{
    char buf[32];
    int id;

    printf("\nВведите ID контакта: ");
    read_line(buf, sizeof(buf));
    if (sscanf(buf, "%d", &id) != 1) {
        printf("Некорректный ввод ID!\n");
        return;
    }

    const Contact *c = contact_find_by_id(root, id);
    if (c) {
        print_contact_card(c);
    } else {
        printf("Контакт с ID %d не найден.\n", id);
    }
}

static void ui_delete(Contact **root, size_t *count)
{
    char buf[32];
    int id;

    printf("\nВведите ID контакта для удаления: ");
    read_line(buf, sizeof(buf));
    if (sscanf(buf, "%d", &id) != 1) {
        printf("Некорректный ввод ID!\n");
        return;
    }

    if (contact_delete(root, count, id) == 0) {
        printf("Контакт с ID %d успешно удален.\n", id);
        handle_auto_balance_notify(root, *count);
    } else {
        printf("Контакт с ID %d не найден.\n", id);
    }
}

static void ui_edit(Contact **root, size_t count)
{
    char buf[32];
    int id;

    printf("\nВведите ID контакта для редактирования: ");
    read_line(buf, sizeof(buf));
    if (sscanf(buf, "%d", &id) != 1) {
        printf("Некорректный ввод ID!\n");
        return;
    }

    const Contact *c = contact_find_by_id(*root, id);
    if (!c) {
        printf("Контакт с ID %d не найден.\n", id);
        return;
    }

    printf("\nРедактирование контакта ID %d (ID изменить нельзя!)\n", id);
    printf("1. Изменить ФИО (с авто-перемещением в BST)\n");
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
                contact_update_name(root, id, format, &upConName);
                printf("ФИО обновлено. Узел перемещен на новую позицию в дереве.\n");
                handle_auto_balance_notify(root, count);
            } else {
                printf("Изменения не внесены.\n");
            }
            break;
        }
        case 2: {
            char phone[MAX_PHONE_LEN];
            printf("Введите новый номер телефона: ");
            read_line(phone, sizeof(phone));
            if (contact_add_phone(*root, id, phone) == 0) {
                printf("Номер добавлен.\n");
            } else {
                printf("Ошибка: превышен лимит телефонов (%d).\n", MAX_PHONES);
            }
            break;
        }
        case 3: {
            if (c->phoneCount == 0) {
                printf("У контакта нет номеров телефонов.\n");
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
                contact_remove_phone(*root, id, (size_t)(idx - 1));
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

            if (contact_add_address(*root, id, &a) == 0) {
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
                contact_remove_address(*root, id, (size_t)(idx - 1));
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

static void ui_manual_balance(Contact **root, size_t count)
{
    if (!root || !(*root) || count <= 2) {
        printf("\nБалансировка не требуется (в дереве меньше 3 узлов).\n");
        return;
    }

    int h_before = contact_get_height(*root);
    if (contact_balance(root, count) == 0) {
        int h_after = contact_get_height(*root);
        printf("\nРучная балансировка успешно выполнена!\n");
        printf("Высота дерева: %d -> %d (узлов: %zu)\n", h_before, h_after, count);
    } else {
        printf("Ошибка выделения памяти при балансировке.\n");
    }
}

void run_phonebook_ui(void)
{
    Contact *root = NULL;
    size_t count = 0;
    int next_id = 1;

    char menu_buf[16];
    int choice = -1;

    while (1) {
        printf("\n============================================\n");
        printf("    ТЕЛЕФОННАЯ КНИГА (Бинарное дерево BST)  \n");
        printf("    Узлов: %-4zu | Высота: %-2d | Корень: %s\n",
               count, contact_get_height(root),
               root ? root->name.lastName : "(пусто)");
        printf("============================================\n");
        printf("1. Показать список (LTR, по алфавиту)\n");
        printf("2. Показать список (TLR, прямой обход)\n");
        printf("3. Показать список (LRT, обратный обход)\n");
        printf("4. Отобразить дерево в консоли (с h и bf)\n");
        printf("5. Добавить контакт\n");
        printf("6. Редактировать контакт\n");
        printf("7. Удалить контакт\n");
        printf("8. Сбалансировать дерево вручную\n");
        printf("9. Загрузить 8 тестовых контактов (Preset)\n");
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
            case 1: ui_view_traversal(root, count, ORDER_LTR, "Обход LTR (Симметричный, алфавит)"); break;
            case 2: ui_view_traversal(root, count, ORDER_TLR, "Обход TLR (Прямой обход корней)"); break;
            case 3: ui_view_traversal(root, count, ORDER_LRT, "Обход LRT (Обратный обход)"); break;
            case 4: contact_print_tree(root); break;
            case 5: ui_add_contact(&root, &count, &next_id); break;
            case 6: ui_edit(&root, count); break;
            case 7: ui_delete(&root, &count); break;
            case 8: ui_manual_balance(&root, count); break;
            case 9:
                contact_load_presets(&root, &count, &next_id);
                printf("\nЗагружено 8 тестовых контактов!\n");
                break;
            default:
                printf("Неверный пункт меню!\n");
                break;
        }
    }

    contact_free_all(&root, &count);
}