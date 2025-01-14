#include <complex.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gtk/gtkcssprovider.h>
#include <gtk/gtkshortcut.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iso646.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include "exampleapp.h"

#define MAX_NUMBERS 1024

// Добавляем цвета
const char *colors[] = {
    "\033[31m",
    "\033[32m",
    "\033[32m",
    "\033[33m",
    "\033[34m",
    "\033[35m",
    "\033[36m",
};

// Сброс цвета
const char *reset = "\033[0m";

// Глобальные переменные 
static GtkWidget *button;
static GtkWidget *display_label;
static gchar *current_input = NULL;
static GtkWidget *textview;
static GtkTextBuffer *buffer;

static void
quit_activated(GSimpleAction *action, GVariant *parametr, gpointer app)
{
    g_application_quit(G_APPLICATION(app));
}

static void
preferences_activated(GSimpleAction *action,
                      GVariant      *parametr,
                      gpointer      app)
{
}

static GActionEntry app_entries[] =
{
    { "preferences", preferences_activated, NULL, NULL, NULL },
    { "quit", quit_activated, NULL, NULL, NULL }
};

// Проверка строки на число
bool isNumber (const gchar *str) {
    if (*str == '\0') return false;
    gchar *endptr;
    strtod(str, &endptr);
    return *endptr == '\0';
}

// Автомотическое Форматирование результата
void auto_format(double value, char *buffer, size_t buffer_size) {
    if (value == (int)value) {
        g_snprintf(buffer, buffer_size, "%.0f", value);
    } else {
        // Проверяем сколько знаков после запятой нужно
        int precision = 9;
        char temp_buffer[256];
        for (int i = 1; i <= precision; i++) {
            g_snprintf(temp_buffer, sizeof(temp_buffer), "%.*f", i, value);
            if (atof(temp_buffer) == value) {
                g_snprintf(buffer, buffer_size, "%s", temp_buffer);
                return;
            }
        }
        // Если достигнут лимит точности после точки выводим
        g_snprintf(buffer, buffer_size, "%.9f", value);
    }
}

char *process_expression(const char *expr)
{
    if (!expr) {
        return g_strdup("");
    }
   
    char *result_expr = g_strdup(expr);
    gchar *ptr = result_expr;

    while((ptr = g_strstr_len(ptr, -1, "÷"))) {
        *ptr = '/';
         memmove(ptr + 1, ptr + 2, strlen(ptr + 2) + 1);
    }
    ptr = result_expr;
    while ((ptr = g_strstr_len(ptr, -1, "×"))) {
        *ptr = '*';
         memmove(ptr + 1, ptr + 2, strlen(ptr + 2) + 1);
    }
    return result_expr;
}

double calculate(const char *expression) {
    double stack_values[MAX_NUMBERS] = {0};
    double stack_operators[MAX_NUMBERS] = {0};
    int value_count = 0, operator_count = 0;
    const char *current = expression;

    if (expression == NULL or *expression == '\0') {
        gtk_text_buffer_set_text(buffer, "ERROR!!!", -1);
        return NAN;
    }

    // Парсинг выражения
    while (*current) {
        while (isspace(*current)) current++;

        char *end_ptr;
        double num = strtod(current, &end_ptr);\

        
        if (*end_ptr == '%' and (isdigit(*(end_ptr + 1)))) {
            return NAN;
        }

        while (*end_ptr == '%') {
            num /= 100.0;
            end_ptr++;
        }

        if (end_ptr == current) {
            gtk_text_buffer_set_text(buffer, "Некорректное выражения", -1);
            return 1;
        }

        if (value_count < MAX_NUMBERS) {
            stack_values[value_count++] = num;
        }
        current = end_ptr;

        // Пропуск пробелов
        while (isspace(*current)) current++;

        // Чтение оператора
        if (*current and strchr("+-*/%", *current)) {
            if (*current == '%' and (isspace(*(current + 1)))) {
                return NAN;
            }
            if (operator_count < MAX_NUMBERS) {
                stack_operators[operator_count++] = *current;
            }
            current++;
        }
    }

    // Оброботка операций с приоритетом (*, /)
    for (int i = 0; i < operator_count; i++) {
        if (stack_operators[i] == '*' or stack_operators[i] == '/' or stack_operators[i] == '%') {
            if (stack_operators[i] == '/' and stack_values[i + 1] == 0) {
                gtk_text_buffer_set_text(buffer, "Ошибка Деление на ноль", -1);
                return NAN;
            }
            stack_values[i] = (stack_operators[i] == '*')
                              ? stack_values[i] * stack_values[i + 1]
                              : (stack_operators[i] == '/')
                                    ? stack_values[i] / stack_values[i + 1]
                                    : fmod(stack_values[i], stack_values[i + 1]);
            // Сдвиг массива значений
            for (int j = i + 1; j < value_count; j++) {
                stack_values[j] = stack_values[j + 1];
            }
            // Сдвиг массива операторов 
            for (int j = i; j < operator_count; j++) {
                stack_operators[j] = stack_operators[j + 1];
            }
            value_count--;
            operator_count--;
            i--;
        }
    }

    // Обработка оставшихся операций (+, -)
    double result = stack_values[0];
    for (int i = 0; i < operator_count; i++) {
        result = stack_operators[i] == '+'
                     ? result + stack_values[i + 1]
                     : result - stack_values[i + 1];
    }
    return result;
}

static void on_button_click(GtkButton *button, gpointer user_data) 
{
    const char *button_label = gtk_button_get_label(button);

    GtkTextIter end_iter, start_iter;
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gchar *current_text = gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, FALSE);
    //const char *current_text = gtk_label_get_text(GTK_LABEL(display_label)) ?: "";

    // Если нажата кнопка "С" очищаем экран
    if (g_strcmp0(button_label, "C") == 0) {
        gtk_text_buffer_set_text(buffer, "", -1);
        g_free(current_text);
        g_free(current_input);
        return;
    }

    if (current_text == NULL) {
        current_text = g_strdup("");
    }

    // Если нажата кнопка - "=" то выполняем вычесление
    if (g_strcmp0(button_label, "=") == 0) {
        if (*current_text == '\0') {
            gtk_text_buffer_set_text(buffer, "Пустое выражение", -1);
            g_free(current_text);
            return;
        }

        if (strstr(current_text, "²") != NULL) {
            if (strlen(current_text) > MAX_NUMBERS) {
                gtk_text_buffer_set_text(buffer, "Ошибка Перевышен лимит символов", -1);
                g_free(current_text);
                return;
            }
            char *base = g_strdup(current_text);
            char *ptr = strstr(base, "²");
            if (ptr != NULL) *ptr = '\0';

            if (ptr != NULL and isNumber(base)) {
                double num = atof(base);
                double squared = num * num;
                char result_text[256];
                auto_format(squared, result_text, sizeof(result_text));
                gtk_text_buffer_set_text(buffer, result_text, -1);
            } else {
                gtk_text_buffer_set_text(buffer, "Ошибка возведения в квадрат", -1);
            }
            g_free(base);
        } else if (strstr(current_text, "√") != NULL) {
            char *root_part = g_strdup(current_text);
            char *ptr = strstr(root_part, "√");
            if (ptr != NULL) {
                memmove(root_part, ptr + 3, strlen(ptr + 3) + 1);
            }

            if (isNumber(root_part)) {
                double number = atof(root_part);
                double sqlrt_result = sqrt(number);
                char result_text[256];
                auto_format(sqlrt_result, result_text, sizeof(result_text));
                gtk_text_buffer_set_text(buffer, result_text, -1);
            } else {
                gtk_text_buffer_set_text(buffer, "Некорректное выражение", -1);
            }
            g_free(root_part);
        } else {
            char *processed_expr = process_expression(current_text);
            double result = calculate(processed_expr);
            if (isnan(result)) {
                gtk_text_buffer_set_text(buffer, "Некорректное выражение", -2);
            } else {
                char result_text[256];
                auto_format(result, result_text, sizeof(result_text));
                gtk_text_buffer_set_text(buffer, result_text, -1);
                g_free(processed_expr);
            }
        }
        g_free(current_text);
        return;
    }

    if (g_strcmp0(button_label, "√") == 0 or 
        g_strcmp0(button_label, "÷") == 0 or 
        g_strcmp0(button_label, "×") == 0 or
        g_strcmp0(button_label, "²") == 0 or
        g_strcmp0(button_label, "π") == 0) {
        char *new_text = g_strconcat(current_text, button_label, NULL);
        gtk_text_buffer_set_text(buffer, new_text, -1);
        g_free(new_text);
        g_free(current_text);
        return;
    }

    char *new_text = g_strconcat(current_text, button_label, NULL);
    // Проверка кодировки
    if (g_utf8_validate(new_text, -1, NULL)) {
        gtk_text_buffer_set_text(buffer, new_text, -1);
    } else {
        gtk_text_buffer_set_text(buffer, "Ошибка", -1);
    }
    g_free(current_text);
    g_free(new_text);
}

static void style_css() {
    gtk_widget_add_css_class(display_label, "calc-display");
    gtk_widget_add_css_class(display_label, "result_display");
    gtk_widget_add_css_class(display_label, "button-number");
    const char *css =
        ".calc-display {"
        "   padding: 25px;"
        "   color: white;"
        "   font-size: 25px;"
        "   background-color: rgba(43, 43, 43, 0.5);"
        //"   border: 2px solid #fa8231;"
        "   border-radius: 5px;"
        "   min-height: 120px;"
        //"   text-decoration: underline;"
        "}"
        ".button-calc {"
        "   background-color: #545454;" 
        "   color: white;"
        "   font-size: 15px;"
        "   border-radius: 5px;"
        "   box-shadow: none;"
        "   border: none;"
        "   min-width: 100px;"
        "   min-height: 40px;"
        "   margin: 2px;"
        "   padding: 0px;"
        "}"
        ".button-number * {"
        "   background-color: rgba(136, 136, 136, 0.5);"
        //"   background-color: alpha(white, .10);"
        "   color: inherit;"
        "   border-radius: 5px;"
        "   border: none;"
        "   box-shadow: none;"
        "   min-width: 100px;"
        "   min-height: 40px;"
        //"   margin-left: 12px;"
        //"   margin-right: 12px;"
        //"   margin-top: 12px;"
        //"   margin-bottom: 12px;"
        "   transition: 200ms cubic-bezier(0.25, 0.46, 0.45, 0.94);"
        "}"
        ".button-equal {"
        "   background-color: transparent;"
        "   color: white;"
        "   font-weight: bold;"
        "   min-height: 60px;"
        "   min-width: 60px;"
        "   border-radius: 5px;"
        "   box-shadow: none;"
        "   border: none;"
        "   padding: 0px;"
        "}"
        ".button-equal * {"
        "   background-color: #ff7c00;"
        "   min-height: 60px;"
        "   min-width: 100px;"
        "   border-radius: 5px;"
        "   color: inherit;"
        "   border: none;"
        "   box-shadow: none;"
        "}"
        "#result_display {"
        "   background-color: #fa8231;"
        "   font-size: 20px;"
        "   color: white;"
        "   min-width: 50px;"
        "   min-height: 100px;"
        "   font-weight: bold;"
        "   border-radius: 5px;"
        "}"
        "textview {"
        "   padding: 0px 12px;"
        "   padding-bottom: 8px;"
        "   padding-left: 0px;"
        "   font-size: 1.4em;"
        "   border-top: 1px solid @card_shade_color;"
        "   border-top-color: @borders;"
        "   color: inherit;"
        "   font-weight: bold;"
        "}"
        ".button-equal:hover {"
        "   background-color: #ff9f45;"
        "}";

    GtkCssProvider *provide = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provide, css);

    GdkDisplay *display = gdk_display_get_default();
    if (GDK_IS_DISPLAY(display)) {
        gtk_style_context_add_provider_for_display(
                display,
                GTK_STYLE_PROVIDER(provide),
                GTK_STYLE_PROVIDER_PRIORITY_USER
        );
    }

    g_object_unref(provide);
}

static void activate(GtkApplication *app, gpointer user_data) {

    g_print("Aplication activeted...\n");

    GtkWidget *window = gtk_application_window_new(app);
    // Название окна
    gtk_window_set_title(GTK_WINDOW(window), "Calculator");
    gtk_window_maximize(GTK_WINDOW(window));
    
    // Глобальный Контейнер main
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(main_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(main_box, GTK_ALIGN_END);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Создаем Контейнер для прокрутки поля ввода 
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_box_append(GTK_BOX(main_box), scrolled_window);
    
    textview = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), TRUE);
    gtk_text_view_set_accepts_tab(GTK_TEXT_VIEW(textview), FALSE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(textview), 15);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(textview), 8);
    gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(textview), 2);

    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview), TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), textview);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

    // Дисплей
    display_label = gtk_label_new("");
    gtk_widget_set_halign(display_label, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(display_label, TRUE);
    gtk_widget_set_vexpand(display_label, FALSE);
    
    // result display
    gtk_box_append(GTK_BOX(main_box), display_label);

    // Контейнер для кнопок
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(button_box, GTK_ALIGN_END);
    gtk_widget_set_vexpand(button_box, FALSE);
    gtk_box_append(GTK_BOX(main_box), button_box);
    
    // Сетка для кнопок
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(main_box), grid);

    // Создаем кнопки
    char *buttons[] = {
        "C", "(", ")", "mod", "π",
        "7", "8", "9", "÷", "√",
        "4", "5", "6", "×", "²",
        "1", "2", "3", "-", "=",
        "0", ",", "%", "+", NULL,
    };

    int i = 0, row = 0, col = 0;

            // Проходим циклом while по кнопках
            while (buttons[i] != NULL) {
                button = gtk_button_new_with_label(buttons[i]);
                gtk_widget_set_hexpand(button, TRUE);
                gtk_widget_set_vexpand(button, TRUE);
                if (g_strcmp0(buttons[i], "=") == 0) {
                    gtk_widget_add_css_class(button, "button-equal");
                    gtk_grid_attach(GTK_GRID(grid), button, col, row + 1, 1, 10);
                } else {
                    gtk_widget_add_css_class(button, "button-calc");

                    // Добавляем класс для числовых кнопок
                    if (g_ascii_isdigit(buttons[i][0])) {
                        gtk_widget_add_css_class(button, "button-number");
                    }

                    gtk_grid_attach(GTK_GRID(grid), button, col, row + 1, 1, 1);
                }
                g_signal_connect(button, "clicked", G_CALLBACK(on_button_click), NULL);             

                col++;
            if (col > 4) {
                col = 0;
                row++;
            }
            i++;
        }

    style_css();
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    int Choice;
    
    // Выбор перед тем как работать
    printf("1-Calculator:\n2-Exit:\n");

    while (1) {
        char input[100];
        scanf("%s", input);
        if (isNumber(input)) {
            Choice = atoi(input);
            if (Choice == 1) {
                break;
            } else if (Choice == 2) {
                return 0;
            }
        } else {
            printf("Invalid input. Please input %s1 to Calculator, %s2 to Exit: %s", colors[1], colors[0], reset);
            while (getchar() != '\n');
        }
    }

    GtkApplication *app;
    int status;
    
    app = gtk_application_new("Calculator.FLIP", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION (app), argc, argv);
    g_object_unref(app);
    return status;
    return 0;
}
