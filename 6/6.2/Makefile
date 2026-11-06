CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g
FPIC = -fPIC

# Имя динамической библиотеки
LIB_SHARED = libphonebook.so
LIB_OBJS = phone_book.o

# Исполняемый файл
TARGET = phonebook_shared
APP_OBJS = main.o user_interface.o

.PHONY: all clean fclean re

all: $(TARGET)

# 1. Компиляция кода списка с обязательным флагом -fPIC
$(LIB_OBJS): phone_book.c phone_book.h
	$(CC) $(CFLAGS) $(FPIC) -c $< -o $@

# 2. Создание динамической библиотеки .so флажком -shared
$(LIB_SHARED): $(LIB_OBJS)
	$(CC) -shared $(LIB_OBJS) -o $@

# 3. Компиляция модулей UI и main
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 4. Линковка бинарника с библиотекой и жесткой привязкой пути через $ORIGIN
$(TARGET): $(APP_OBJS) $(LIB_SHARED)
	$(CC) $(APP_OBJS) -L. -lphonebook -Wl,-rpath,'$$ORIGIN' -o $@
	@echo "Удаление промежуточных объектных файлов..."
	rm -f $(LIB_OBJS) $(APP_OBJS)

clean:
	rm -f $(LIB_OBJS) $(APP_OBJS)

fclean: clean
	rm -f $(LIB_SHARED) $(TARGET)

re: fclean all