CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g
TARGET = ip_simulation

# Список исходных файлов проекта
SRCS = main.c ip_calc.c user_interface.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean fclean re

# Основная цель
all: $(TARGET)

# Линковка исполняемого файла и автоматическое удаление .o файлов сразу после сборки
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)
	@echo "Удаление промежуточных файлов..."
	rm -f $(OBJS)

# Правило компиляции .c файлов в .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Ручная очистка объектных файлов (если сборка прервалась с ошибкой)
clean:
	rm -f $(OBJS)

# Полная очистка: удаление объектных файлов и исполняемого бинарника
fclean: clean
	rm -f $(TARGET)

# Полная пересборка с нуля
re: fclean all