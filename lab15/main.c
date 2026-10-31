#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_MEMORY_ERROR
} status_code_t;

typedef struct {
    FILE *stream;
    char *buffer;
    size_t count;
} OutWriter;

static void write_char(OutWriter *w, char c) {
    if (w->stream != NULL) {
        fputc(c, w->stream);
    } else if (w->buffer != NULL) {
        w->buffer[w->count] = c;
    }
    w->count++;
}

static void write_str(OutWriter *w, const char *s) {
    if (s == NULL) {
        s = "(null)";
    }
    while (*s != '\0') {
        write_char(w, *s);
        s++;
    }
}

static void format_roman(OutWriter *w, int num) {
    if (num == 0) {
        write_str(w, "0");
        return;
    }
    if (num < 0) {
        write_char(w, '-');
        if (num == INT_MIN) {
            write_str(w, "OVERFLOW");
            return;
        }
        num = -num;
    }
    struct {
        int val;
        const char *sym;
    } roman_table[] = {
        {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"},
        {100, "C"},  {90, "XC"},  {50, "L"},  {40, "XL"},
        {10, "X"},   {9, "IX"},   {5, "V"},   {4, "IV"},
        {1, "I"}
    };
    for (size_t i = 0; i < sizeof(roman_table) / sizeof(roman_table[0]); ++i) {
        while (num >= roman_table[i].val) {
            write_str(w, roman_table[i].sym);
            num -= roman_table[i].val;
        }
    }
}

static void format_zeckendorf(OutWriter *w, unsigned int num) {
    if (num == 0) {
        write_str(w, "01");
        return;
    }
    unsigned int fib[48];
    fib[0] = 1;
    size_t n_fib = 1;
    if (num >= 2) {
        fib[1] = 2;
        n_fib = 2;
        while (1) {
            unsigned int next = fib[n_fib - 1] + fib[n_fib - 2];
            if (next < fib[n_fib - 1] || next > num) {
                break;
            }
            fib[n_fib++] = next;
        }
    }

    char bits[64];
    for (size_t i = 0; i < n_fib; ++i) {
        bits[i] = '0';
    }

    unsigned int rem = num;
    for (size_t i = n_fib; i > 0; --i) {
        size_t idx = i - 1;
        if (rem >= fib[idx]) {
            bits[idx] = '1';
            rem -= fib[idx];
        }
    }

    for (size_t i = 0; i < n_fib; ++i) {
        write_char(w, bits[i]);
    }
    write_char(w, '1');
}

static void format_base(OutWriter *w, int num, int base, int uppercase) {
    if (base < 2 || base > 36) {
        base = 10;
    }
    if (num == 0) {
        write_char(w, '0');
        return;
    }

    unsigned int uval;
    if (num < 0) {
        write_char(w, '-');
        uval = 0U - (unsigned int)num;
    } else {
        uval = (unsigned int)num;
    }

    char buf[64];
    size_t len = 0;
    const char *digits = uppercase ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   : "0123456789abcdefghijklmnopqrstuvwxyz";
    while (uval > 0) {
        buf[len++] = digits[uval % (unsigned int)base];
        uval /= (unsigned int)base;
    }

    for (size_t i = len; i > 0; --i) {
        write_char(w, buf[i - 1]);
    }
}

static void format_from_base(OutWriter *w, const char *str, int base, int uppercase) {
    if (base < 2 || base > 36) {
        base = 10;
    }
    if (str == NULL || *str == '\0') {
        write_str(w, "0");
        return;
    }

    int sign = 1;
    const char *p = str;
    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        sign = -1;
        p++;
    }

    if (*p == '\0') {
        write_str(w, "0");
        return;
    }

    long long res = 0;
    int valid = 1;
    while (*p != '\0') {
        int val = -1;
        if (*p >= '0' && *p <= '9') {
            val = *p - '0';
        } else if (uppercase && *p >= 'A' && *p <= 'Z') {
            val = *p - 'A' + 10;
        } else if (!uppercase && *p >= 'a' && *p <= 'z') {
            val = *p - 'a' + 10;
        } else {
            valid = 0;
            break;
        }

        if (val < 0 || val >= base) {
            valid = 0;
            break;
        }

        res = res * base + val;
        p++;
    }

    if (!valid) {
        write_str(w, "ERROR");
    } else {
        res *= sign;
        char buf[32];
        snprintf(buf, sizeof(buf), "%lld", res);
        write_str(w, buf);
    }
}

static void format_memory_dump(OutWriter *w, const void *ptr, size_t size) {
    const unsigned char *bytes = (const unsigned char *)ptr;
    for (size_t i = 0; i < size; ++i) {
        if (i > 0) {
            write_char(w, ' ');
        }
        for (int b = 7; b >= 0; --b) {
            write_char(w, (bytes[i] & (1 << b)) ? '1' : '0');
        }
    }
}

static int is_std_spec_letter(char c) {
    return (c == 'd' || c == 'i' || c == 'u' || c == 'f' || c == 's' ||
            c == 'c' || c == 'x' || c == 'X' || c == 'p' || c == '%');
}

static int over_vformat(OutWriter *w, const char *format, va_list args) {
    if (format == NULL) {
        return -1;
    }
    const char *p = format;
    while (*p != '\0') {
        if (*p != '%') {
            write_char(w, *p);
            p++;
            continue;
        }
        p++;
        if (*p == '\0') {
            break;
        }
        if (p[0] == 'R' && p[1] == 'o') {
            int num = va_arg(args, int);
            format_roman(w, num);
            p += 2;
        } else if (p[0] == 'Z' && p[1] == 'r') {
            unsigned int num = va_arg(args, unsigned int);
            format_zeckendorf(w, num);
            p += 2;
        } else if (p[0] == 'C' && p[1] == 'v') {
            int num = va_arg(args, int);
            int base = va_arg(args, int);
            format_base(w, num, base, 0);
            p += 2;
        } else if (p[0] == 'C' && p[1] == 'V') {
            int num = va_arg(args, int);
            int base = va_arg(args, int);
            format_base(w, num, base, 1);
            p += 2;
        } else if (p[0] == 't' && p[1] == 'o') {
            const char *str = va_arg(args, const char *);
            int base = va_arg(args, int);
            format_from_base(w, str, base, 0);
            p += 2;
        } else if (p[0] == 'T' && p[1] == 'O') {
            const char *str = va_arg(args, const char *);
            int base = va_arg(args, int);
            format_from_base(w, str, base, 1);
            p += 2;
        } else if (p[0] == 'm' && p[1] == 'i') {
            int32_t val = (int32_t)va_arg(args, int);
            format_memory_dump(w, &val, sizeof(int32_t));
            p += 2;
        } else if (p[0] == 'm' && p[1] == 'u') {
            uint32_t val = (uint32_t)va_arg(args, unsigned int);
            format_memory_dump(w, &val, sizeof(uint32_t));
            p += 2;
        } else if (p[0] == 'm' && p[1] == 'd') {
            double val = va_arg(args, double);
            format_memory_dump(w, &val, sizeof(double));
            p += 2;
        } else if (p[0] == 'm' && p[1] == 'f') {
            float val = (float)va_arg(args, double);
            format_memory_dump(w, &val, sizeof(float));
            p += 2;
        } else {
            char spec[32];
            size_t slen = 0;
            spec[slen++] = '%';
            while (*p != '\0' && !is_std_spec_letter(*p) && slen < 30) {
                spec[slen++] = *p++;
            }
            if (*p != '\0') {
                char letter = *p;
                spec[slen++] = *p++;
                spec[slen] = '\0';

                char tmp[128];
                if (letter == '%') {
                    write_char(w, '%');
                } else if (letter == 'd' || letter == 'i') {
                    int val = va_arg(args, int);
                    snprintf(tmp, sizeof(tmp), spec, val);
                    write_str(w, tmp);
                } else if (letter == 'u' || letter == 'x' || letter == 'X') {
                    unsigned int val = va_arg(args, unsigned int);
                    snprintf(tmp, sizeof(tmp), spec, val);
                    write_str(w, tmp);
                } else if (letter == 'f') {
                    double val = va_arg(args, double);
                    snprintf(tmp, sizeof(tmp), spec, val);
                    write_str(w, tmp);
                } else if (letter == 's') {
                    const char *val = va_arg(args, const char *);
                    write_str(w, val);
                } else if (letter == 'c') {
                    int val = va_arg(args, int);
                    write_char(w, (char)val);
                } else if (letter == 'p') {
                    void *val = va_arg(args, void *);
                    snprintf(tmp, sizeof(tmp), spec, val);
                    write_str(w, tmp);
                }
            }
        }
    }
    return (int)w->count;
}

int oversprintf(char *str, const char *format, ...) {
    if (str == NULL || format == NULL) {
        return -1;
    }
    OutWriter w = {NULL, str, 0};
    va_list args;
    va_start(args, format);
    int res = over_vformat(&w, format, args);
    va_end(args);
    str[w.count] = '\0';
    return res;
}

int overfprintf(FILE *stream, const char *format, ...) {
    if (stream == NULL || format == NULL) {
        return -1;
    }
    OutWriter w = {stream, NULL, 0};
    va_list args;
    va_start(args, format);
    int res = over_vformat(&w, format, args);
    va_end(args);
    return res;
}

static int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

static int parse_int(const char *str, int *out_val) {
    if (str == NULL || out_val == NULL || *str == '\0') {
        return 0;
    }
    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    if (*str == '\0') {
        return 0;
    }
    long long res = 0;
    while (*str) {
        if (!is_digit(*str)) {
            return 0;
        }
        int d = *str - '0';
        res = res * 10 + d;
        if (sign == 1 && res > INT_MAX) {
            return 0;
        }
        if (sign == -1 && (-res) < INT_MIN) {
            return 0;
        }
        str++;
    }
    *out_val = (int)(res * sign);
    return 1;
}

static int parse_uint(const char *str, unsigned int *out_val) {
    if (str == NULL || out_val == NULL || *str == '\0') {
        return 0;
    }
    if (*str == '+') {
        str++;
    }
    if (*str == '\0') {
        return 0;
    }
    unsigned long long res = 0;
    while (*str) {
        if (!is_digit(*str)) {
            return 0;
        }
        res = res * 10 + (*str - '0');
        if (res > UINT_MAX) {
            return 0;
        }
        str++;
    }
    *out_val = (unsigned int)res;
    return 1;
}

static int parse_double(const char *str, double *out_val) {
    if (str == NULL || out_val == NULL || *str == '\0') {
        return 0;
    }
    int dot_count = 0;
    const char *p = str;
    if (*p == '+' || *p == '-') {
        p++;
    }
    if (*p == '\0') {
        return 0;
    }
    while (*p) {
        if (*p == '.') {
            dot_count++;
            if (dot_count > 1) {
                return 0;
            }
        } else if (!is_digit(*p)) {
            return 0;
        }
        p++;
    }

    double sign = 1.0;
    p = str;
    if (*p == '-') {
        sign = -1.0;
        p++;
    } else if (*p == '+') {
        p++;
    }
    double integer = 0.0;
    while (*p && *p != '.') {
        integer = integer * 10.0 + (*p - '0');
        p++;
    }
    double frac = 0.0;
    double div = 1.0;
    if (*p == '.') {
        p++;
        while (*p) {
            frac = frac * 10.0 + (*p - '0');
            div *= 10.0;
            p++;
        }
    }
    *out_val = sign * (integer + frac / div);
    return 1;
}

void run_demonstration(void) {
    printf("=== ДЕМОНСТРАЦИЯ overfprintf И oversprintf ===\n\n");
    char buffer[512];

    oversprintf(buffer, "%%Ro: Римское 1994 = %Ro, 4 = %Ro, 3999 = %Ro", 1994, 4, 3999);
    printf("[oversprintf] %s\n", buffer);

    oversprintf(buffer, "%%Zr: Цекендорф 10 = %Zr, 1 = %Zr, 25 = %Zr", 10, 1, 25);
    printf("[oversprintf] %s\n", buffer);

    oversprintf(buffer, "%%Cv / %%CV: 255 в 16-ричной: lower=%Cv, upper=%CV", 255, 16, 255, 16);
    printf("[oversprintf] %s\n", buffer);

    oversprintf(buffer, "%%to / %%TO: 'ff' в base 16 = %to, 'FF' в base 16 = %TO", "ff", 16, "FF", 16);
    printf("[oversprintf] %s\n", buffer);

    oversprintf(buffer, "%%mi (знаковое 4 байта): %mi", 1);
    printf("[oversprintf] %s\n", buffer);
    oversprintf(buffer, "%%mu (беззнаковое 4 байта): %mu", 1U);
    printf("[oversprintf] %s\n", buffer);

    oversprintf(buffer, "%%mf (float): %mf", 1.0f);
    printf("[oversprintf] %s\n", buffer);
    oversprintf(buffer, "%%md (double): %md", 1.0);
    printf("[oversprintf] %s\n", buffer);

    FILE *fout = fopen("overfprintf_test.txt", "w");
    if (fout != NULL) {
        overfprintf(fout, "Roman: %Ro\nZeck: %Zr\nBase: %Cv / %CV\nMemory: %mi\n", 1994, 10, 255, 16, 255, 16, 5);
        fclose(fout);
        printf("\n[overfprintf] Запись в файл 'overfprintf_test.txt' успешно выполнена.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1 || (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'd' && argv[1][2] == 'e')) {
        run_demonstration();
        return 0;
    }

    const char *flag = argv[1];
    if (flag[0] != '-' && flag[0] != '/') {
        printf("Ошибка: неверный формат флага. Ожидается -<флаг>.\n");
        return 1;
    }
    const char *spec = flag + 1;

    char buf[512];

    if (spec[0] == 'R' && spec[1] == 'o' && spec[2] == '\0') {
        if (argc != 3) {
            printf("Ошибка: для флага -Ro ожидается 1 аргумент: целое число int.\n");
            return 1;
        }
        int val = 0;
        if (!parse_int(argv[2], &val)) {
            printf("Ошибка: передан некорректный аргумент (ожидалось целое число int без переполнения).\n");
            return 1;
        }
        oversprintf(buf, "Результат: %Ro", val);
        printf("%s\n", buf);
    } else if (spec[0] == 'Z' && spec[1] == 'r' && spec[2] == '\0') {
        if (argc != 3) {
            printf("Ошибка: для флага -Zr ожидается 1 аргумент: целое число unsigned int.\n");
            return 1;
        }
        unsigned int val = 0;
        if (!parse_uint(argv[2], &val)) {
            printf("Ошибка: передан некорректный аргумент (ожидалось неотрицательное целое число без переполнения).\n");
            return 1;
        }
        oversprintf(buf, "Результат: %Zr", val);
        printf("%s\n", buf);
    } else if ((spec[0] == 'C' && (spec[1] == 'v' || spec[1] == 'V') && spec[2] == '\0')) {
        if (argc != 4) {
            printf("Ошибка: для флага -%s ожидается 2 аргумента: <число int> <основание [2..36]>.\n", spec);
            return 1;
        }
        int val = 0;
        int base = 0;
        if (!parse_int(argv[2], &val) || !parse_int(argv[3], &base)) {
            printf("Ошибка: некорректные аргументы.\n");
            return 1;
        }
        if (spec[1] == 'v') {
            oversprintf(buf, "Результат: %Cv", val, base);
        } else {
            oversprintf(buf, "Результат: %CV", val, base);
        }
        printf("%s\n", buf);
    } else if ((spec[0] == 't' && spec[1] == 'o' && spec[2] == '\0') ||
               (spec[0] == 'T' && spec[1] == 'O' && spec[2] == '\0')) {
        if (argc != 4) {
            printf("Ошибка: для флага -%s ожидается 2 аргумента: <строка> <основание [2..36]>.\n", spec);
            return 1;
        }
        int base = 0;
        if (!parse_int(argv[3], &base)) {
            printf("Ошибка: некорректное основание системы счисления.\n");
            return 1;
        }
        if (spec[0] == 't') {
            oversprintf(buf, "Результат: %to", argv[2], base);
        } else {
            oversprintf(buf, "Результат: %TO", argv[2], base);
        }
        printf("%s\n", buf);
    } else if (spec[0] == 'm' && spec[1] == 'i' && spec[2] == '\0') {
        if (argc != 3) {
            printf("Ошибка: для флага -mi ожидается 1 аргумент: число int.\n");
            return 1;
        }
        int val = 0;
        if (!parse_int(argv[2], &val)) {
            printf("Ошибка: некорректный int.\n");
            return 1;
        }
        oversprintf(buf, "Дамп памяти: %mi", val);
        printf("%s\n", buf);
    } else if (spec[0] == 'm' && spec[1] == 'u' && spec[2] == '\0') {
        if (argc != 3) {
            printf("Ошибка: для флага -mu ожидается 1 аргумент: число unsigned int.\n");
            return 1;
        }
        unsigned int val = 0;
        if (!parse_uint(argv[2], &val)) {
            printf("Ошибка: некорректный unsigned int.\n");
            return 1;
        }
        oversprintf(buf, "Дамп памяти: %mu", val);
        printf("%s\n", buf);
    } else if (spec[0] == 'm' && spec[1] == 'f' && spec[2] == '\0') {
        if (argc != 3) {
            printf("Ошибка: для флага -mf ожидается 1 аргумент: вещественное число float.\n");
            return 1;
        }
        double val = 0.0;
        if (!parse_double(argv[2], &val)) {
            printf("Ошибка: некорректное вещественное число.\n");
            return 1;
        }
        float fval = (float)val;
        oversprintf(buf, "Дамп памяти: %mf", fval);
        printf("%s\n", buf);
    } else if (spec[0] == 'm' && spec[1] == 'd' && spec[2] == '\0') {
        if (argc != 3) {
            printf("Ошибка: для флага -md ожидается 1 аргумент: вещественное число double.\n");
            return 1;
        }
        double val = 0.0;
        if (!parse_double(argv[2], &val)) {
            printf("Ошибка: некорректное вещественное число.\n");
            return 1;
        }
        oversprintf(buf, "Дамп памяти: %md", val);
        printf("%s\n", buf);
    } else {
        printf("Ошибка: неизвестный флаг '-%s'.\n", spec);
        return 1;
    }

    return 0;
}