CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g -I.
SANFLAGS = -fsanitize=address,undefined

# Основной исполняемый файл и исходники
TARGET = phonebook
SRCS = main.c phone_book.c user_interface.c
OBJS = $(SRCS:.c=.o)

# Тестовый исполняемый файл и исходники (Unity + ядро)
TEST_TARGET = test_runner
TEST_SRCS = tests/test_phone_book.c tests/unity/unity.c phone_book.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

.PHONY: all clean fclean re test

# Основная цель
all: $(TARGET)

# Линковка основного бинарника и автоудаление .o файлов
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)
	@echo "Удаление промежуточных файлов..."
	rm -f $(OBJS)

# Линковка тестов с санитайзерами и автоудаление .o файлов тестов
$(TEST_TARGET): CFLAGS += $(SANFLAGS)
$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $(TEST_TARGET)
	@echo "Удаление промежуточных файлов тестов..."
	rm -f $(TEST_OBJS)

# Сборка и автоматический запуск тестов
test: $(TEST_TARGET)
	@echo "\n>>> ЗАПУСК ТЕСТОВ СТРУКТУРЫ ДАННЫХ <<<"
	./$(TEST_TARGET)

# Универсальное правило компиляции .c в .o (включая файлы в подпапках tests/)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Ручная очистка всех объектных файлов (если сборка прервалась)
clean:
	rm -f $(OBJS) $(TEST_OBJS)

# Полная очистка: объектные файлы + оба бинарника
fclean: clean
	rm -f $(TARGET) $(TEST_TARGET)

# Полная пересборка с нуля
re: fclean all