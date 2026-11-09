CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g -I.
AR = ar
ARFLAGS = rcs

# Имя статической библиотеки и её модули
LIB_STATIC = libphonebook.a
LIB_OBJS = phone_book.o

# Основной исполняемый файл
TARGET = phonebook_static
APP_OBJS = main.o user_interface.o

# Тестовый раннер и его модули
TEST_TARGET = test_runner
TEST_SRCS = tests/test_phone_book.c tests/unity/unity.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

.PHONY: all clean fclean re test

all: $(TARGET)

# 1. Сборка статической библиотеки архиватором ar
$(LIB_STATIC): $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $^

# 2. Сборка основного бинарника с подключением библиотеки
$(TARGET): $(APP_OBJS) $(LIB_STATIC)
	$(CC) $(CFLAGS) $(APP_OBJS) -L. -lphonebook -o $@
	@echo "Удаление промежуточных файлов приложения..."
	rm -f $(LIB_OBJS) $(APP_OBJS)

# 3. Сборка тестов, слинкованных со статической библиотекой libphonebook.a
$(TEST_TARGET): $(TEST_OBJS) $(LIB_STATIC)
	$(CC) $(CFLAGS) $(TEST_OBJS) -L. -lphonebook -o $@
	@echo "Удаление промежуточных файлов тестов..."
	rm -f $(TEST_OBJS)

# 4. Запуск тестов
test: $(TEST_TARGET)
	@echo "\n>>> ЗАПУСК ТЕСТОВ СТАТИЧЕСКОЙ БИБЛИОТЕКИ (6.1) <<<"
	./$(TEST_TARGET)

# Универсальное правило компиляции .c в .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(LIB_OBJS) $(APP_OBJS) $(TEST_OBJS)

fclean: clean
	rm -f $(LIB_STATIC) $(TARGET) $(TEST_TARGET)

re: fclean all