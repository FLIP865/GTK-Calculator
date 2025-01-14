# Переменные
CC = gcc
CFLAGS = `pkg-config --cflags gtk4` 
LIBS = `pkg-config --libs gtk4`
RESOURCE = src/exampleapp.gresource.c
SRC = src/main.c src/exampleapp.c src/exampleappwin.c
BIN = calculator

# Основное правило
all: $(BIN)
	@echo "Сборка завершана, запуск программы..."
	./$(BIN)

# Генерация файла ресурсов
$(RESOURCE): src/exampleapp.gresource.xml
	@echo "Генерация файла ресурсов"
	@glib-compile-resources src/exampleapp.gresource.xml \
		--sourcedir=src \
		--target=$(RESOURCE) \
		--generate-source

# Компиляция программы
$(BIN): $(SRC) $(RESOURCE)
	@echo "Компиляция программы..."
	@$(CC) $(CFLAGS) $(SRC) $(RESOURCE) -o $(BIN) $(LIBS) -lm

# Очистка
clean:
	rm -rf $(BIN) $(RESOURCE)
	@echo "Очистка завершана"

.PHONY: all clean

