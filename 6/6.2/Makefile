CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g -I.
FPIC = -fPIC

# Имя динамической библиотеки
LIB_SHARED = libphonebook.so
LIB_OBJS = phone_book.o

# Основной исполняемый файл
TARGET = phonebook_shared
APP_OBJS = main.o user_interface.o

# Тестовый раннер и его модули
TEST_TARGET = test_runner
TEST_SRCS = tests/test_phone_book.c tests/unity/unity.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

.PHONY: all clean fclean re test

all: $(TARGET)

# 1. Компиляция кода списка с флагом -fPIC
$(LIB_OBJS): phone_book.c phone_book.h
	$(CC) $(CFLAGS) $(FPIC) -c $< -o $@

# 2. Создание динамической библиотеки .so
$(LIB_SHARED): $(LIB_OBJS)
	$(CC) -shared $(LIB_OBJS) -o $@

# 3. Сборка основного бинарника с привязкой rpath к $ORIGIN
$(TARGET): $(APP_OBJS) $(LIB_SHARED)
	$(CC) $(CFLAGS) $(APP_OBJS) -L. -lphonebook -Wl,-rpath,'$$ORIGIN' -o $@
	@echo "Удаление промежуточных файлов приложения..."
	rm -f $(LIB_OBJS) $(APP_OBJS)

# 4. Сборка тестов, слинкованных с динамической библиотекой libphonebook.so
$(TEST_TARGET): $(TEST_OBJS) $(LIB_SHARED)
	$(CC) $(CFLAGS) $(TEST_OBJS) -L. -lphonebook -Wl,-rpath,'$$ORIGIN' -o $@
	@echo "Удаление промежуточных файлов тестов..."
	rm -f $(TEST_OBJS)

# 5. Запуск тестов
test: $(TEST_TARGET)
	@echo "\n>>> ЗАПУСК ТЕСТОВ ДИНАМИЧЕСКОЙ БИБЛИОТЕКИ (6.2) <<<"
	./$(TEST_TARGET)

# Универсальное правило компиляции .c в .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(LIB_OBJS) $(APP_OBJS) $(TEST_OBJS)

fclean: clean
	rm -f $(LIB_SHARED) $(TARGET) $(TEST_TARGET)

re: fclean all