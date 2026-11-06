CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g
AR = ar
ARFLAGS = rcs

# Имя статической библиотеки
LIB_STATIC = libphonebook.a
LIB_OBJS = phone_book.o

# Исполняемый файл
TARGET = phonebook_static
APP_OBJS = main.o user_interface.o

.PHONY: all clean fclean re

all: $(TARGET)

# 1. Сборка статической библиотеки архиватором ar
$(LIB_STATIC): $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $^

# 2. Сборка исполняемого файла с подключением библиотеки (-L. -lphonebook)
$(TARGET): $(APP_OBJS) $(LIB_STATIC)
	$(CC) $(APP_OBJS) -L. -lphonebook -o $@
	@echo "Удаление промежуточных объектных файлов..."
	rm -f $(LIB_OBJS) $(APP_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(LIB_OBJS) $(APP_OBJS)

fclean: clean
	rm -f $(LIB_STATIC) $(TARGET)

re: fclean all