# GTK-Calculator

GTK-Calculator — это простой калькулятор с графическим интерфейсом, созданный с использованием GTK4. Он поддерживает базовые арифметические операции, возведение в степень, извлечение квадратного корня.

## Функции калькулятора
- Арифметические операции: сложение, вычитание, умножение, деление.
- Возведение в степень (`²`).
- Извлечение квадратного корня (`√`).
- Использование числа π (3.141592654) пока-что не работает
- Очистка экрана (кнопка `C`).

## Зависимости
Для работы калькулятора требуются следующие библиотеки:
- GTK 4
- GLib
- Pango
- Cairo
- Math Library (libm, обычно включена в стандартную библиотеку C)

## Установка зависимостей
### На Ubuntu/Debian
Выполните следующие команды для установки всех необходимых библиотек:
```bash
sudo apt update
sudo apt install -y build-essential pkg-config libgtk-4-dev bear
```
## Установка зависимостей
### На ArchLinux
Выполните следующие команды для установки всех необходимых библиотек:
```bash
sudo pacman -Syu
sudo pacman -S base-devel gtk4 bear
```
# Компиляция 
Выполните следущие команды
```bash
git clone https://github.com/FLIP865/GTK-Calculator/
cd GTK-Calculator
make
```
# GTK-Calculator
Калькулятор пока полностью не доработан в будущем буду добавлять другие функции если вы знает как их реализовать тогда добавляйте 
