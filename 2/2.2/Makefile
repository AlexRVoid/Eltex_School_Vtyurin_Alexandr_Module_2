CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g
TARGET = calculator

# Исходные файлы проекта
SRCS = main.c math_operator.c user_interface.c

# Соответствующие объектные файлы
OBJS = $(SRCS:.c=.o)

.PHONY: all clean fclean re

# Сборка итоговой программы
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Компиляция каждого .c файла в .o файл
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка объектных файлов
clean:
	rm -f $(OBJS)

# Очистка объектных файлов и исполняемого файла
fclean: clean
	rm -f $(TARGET)

# Полная пересборка с нуля
re: fclean all