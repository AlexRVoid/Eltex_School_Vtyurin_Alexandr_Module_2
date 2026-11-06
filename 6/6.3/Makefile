CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g
FPIC = -fPIC

PLUGINS_DIR = plugins
TARGET = calculator

# Список файлов плагинов
PLUGIN_SRCS = $(wildcard $(PLUGINS_DIR)/*.c)
PLUGIN_LIBS = $(PLUGIN_SRCS:.c=.so)

# Основные файлы программы
APP_SRCS = main.c user_interface.c math_operator.c
APP_OBJS = $(APP_SRCS:.c=.o)

.PHONY: all clean fclean re plugins

all: plugins $(TARGET)

# Сборка всех динамических библиотек в папке plugins
plugins: $(PLUGIN_LIBS)

$(PLUGINS_DIR)/%.so: $(PLUGINS_DIR)/%.c math_operator.h
	$(CC) $(CFLAGS) $(FPIC) -shared $< -o $@

# Сборка исполняемого файла с системной библиотекой libdl (-ldl)
$(TARGET): $(APP_OBJS)
	$(CC) $(APP_OBJS) -ldl -o $(TARGET)
	@echo "Удаление промежуточных объектных файлов..."
	rm -f $(APP_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(APP_OBJS)

fclean: clean
	rm -f $(TARGET) $(PLUGINS_DIR)/*.so

re: fclean all