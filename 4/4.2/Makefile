CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g -I.
TARGET = pqueue_app

SRCS = main.c pqueue.c user_interface.c
OBJS = $(SRCS:.c=.o)

TEST_TARGET = test_runner
TEST_SRCS = tests/test_pqueue.c tests/unity/unity.c pqueue.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

.PHONY: all clean fclean re test test_valgrind

all: $(TARGET)

# Сборка основного бинарника с авто-очисткой .o файлов
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)
	@echo "Удаление промежуточных файлов приложения..."
	rm -f $(OBJS)

# Сборка тестового раннера с авто-очисткой .o файлов
$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $(TEST_TARGET)
	@echo "Удаление промежуточных файлов тестов..."
	rm -f $(TEST_OBJS)

# Запуск тестов через Unity
test: $(TEST_TARGET)
	@echo "\n=============================================="
	@echo "   ЗАПУСК UNIT-ТЕСТОВ ОЧЕРЕДИ С ПРИОРИТЕТОМ   "
	@echo "=============================================="
	./$(TEST_TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TEST_OBJS)

fclean: clean
	rm -f $(TARGET) $(TEST_TARGET)

re: fclean all