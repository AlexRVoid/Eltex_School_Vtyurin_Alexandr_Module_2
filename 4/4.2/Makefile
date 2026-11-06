CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g
TARGET = priority_queue

SRCS = main.c pqueue.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean fclean re test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)
	@echo "Удаление промежуточных файлов..."
	rm -f $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

test: $(TARGET)
	./$(TARGET)