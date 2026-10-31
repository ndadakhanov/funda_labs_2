#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_FILE_ERROR
} status_code_t;

typedef struct {
    FILE *stream;
    const char *str;
    size_t pos;
} InReader;

static int reader_getc(InReader *r) {
    if (r->stream != NULL) {
        return fgetc(r->stream);
    }
    if (r->str != NULL) {
        if (r->str[r->pos] != '\0') {
            return (unsigned char)r->str[r->pos++];
        }
        return -1;
    }
    return -1;
}

static void reader_ungetc(InReader *r, int c) {
    if (c == -1) {
        return;
    }
    if (r->stream != NULL) {
        ungetc(c, r->stream);
    } else if (r->str != NULL) {
        if (r->pos > 0) {
            r->pos--;
        }
    }
}

static int is_whitespace(int c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
}

static void skip_whitespace(InReader *r) {
    int c;
    while ((c = reader_getc(r)) != -1) {
        if (!is_whitespace(c)) {
            reader_ungetc(r, c);
            break;
        }
    }
}

static int roman_char_val(char c) {
    switch (c) {
        case 'I': case 'i': return 1;
        case 'V': case 'v': return 5;
        case 'X': case 'x': return 10;
        case 'L': case 'l': return 50;
        case 'C': case 'c': return 100;
        case 'D': case 'd': return 500;
        case 'M': case 'm': return 1000;
        default: return -1;
    }
}

static int scan_roman(InReader *r, int *out_val) {
    skip_whitespace(r);
    int c = reader_getc(r);
    if (c == -1) {
        return 0;
    }

    int sign = 1;
    if (c == '-') {
        sign = -1;
    } else if (c == '+') {
        sign = 1;
    } else {
        reader_ungetc(r, c);
    }

    char buf[64];
    size_t len = 0;
    while ((c = reader_getc(r)) != -1) {
        if (roman_char_val((char)c) != -1) {
            if (len + 1 < sizeof(buf)) {
                buf[len++] = (char)c;
            }
        } else {
            reader_ungetc(r, c);
            break;
        }
    }

    if (len == 0) {
        return 0;
    }

    int total = 0;
    int prev_val = 0;
    for (size_t i = len; i > 0; --i) {
        int v = roman_char_val(buf[i - 1]);
        if (v < prev_val) {
            total -= v;
        } else {
            total += v;
            prev_val = v;
        }
    }

    *out_val = total * sign;
    return 1;
}

static int scan_zeckendorf(InReader *r, unsigned int *out_val) {
    skip_whitespace(r);
    int c = reader_getc(r);
    if (c == -1) {
        return 0;
    }
    reader_ungetc(r, c);

    char buf[64];
    size_t len = 0;
    int prev = -1;

    while ((c = reader_getc(r)) != -1) {
        if (c == '0' || c == '1') {
            if (len + 1 < sizeof(buf)) {
                buf[len++] = (char)c;
            }
            if (prev == '1' && c == '1') {
                break;
            }
            prev = c;
        } else {
            reader_ungetc(r, c);
            break;
        }
    }

    if (len < 2 || buf[len - 1] != '1') {
        return 0;
    }

    unsigned int fib[48];
    fib[0] = 1;
    fib[1] = 2;
    size_t n_fib = 2;
    size_t content_len = len - 1;
    while (n_fib < content_len) {
        fib[n_fib] = fib[n_fib - 1] + fib[n_fib - 2];
        n_fib++;
    }

    unsigned int res = 0;
    for (size_t i = 0; i < content_len; ++i) {
        if (buf[i] == '1') {
            res += fib[i];
        }
    }

    *out_val = res;
    return 1;
}

static int scan_base_int(InReader *r, int *out_val, int base, int uppercase) {
    if (base < 2 || base > 36) {
        base = 10;
    }
    skip_whitespace(r);

    int c = reader_getc(r);
    if (c == -1) {
        return 0;
    }

    int sign = 1;
    if (c == '-') {
        sign = -1;
    } else if (c == '+') {
        sign = 1;
    } else {
        reader_ungetc(r, c);
    }

    long long res = 0;
    size_t digits_read = 0;

    while ((c = reader_getc(r)) != -1) {
        int v = -1;
        if (c >= '0' && c <= '9') {
            v = c - '0';
        } else if (uppercase && c >= 'A' && c <= 'Z') {
            v = c - 'A' + 10;
        } else if (!uppercase && c >= 'a' && c <= 'z') {
            v = c - 'a' + 10;
        }

        if (v >= 0 && v < base) {
            digits_read++;
            res = res * base + v;
            if (sign == 1 && res > INT_MAX) {
                res = INT_MAX;
            }
            if (sign == -1 && -res < INT_MIN) {
                res = -(long long)INT_MIN;
            }
        } else {
            reader_ungetc(r, c);
            break;
        }
    }

    if (digits_read == 0) {
        return 0;
    }

    *out_val = (int)(res * sign);
    return 1;
}

static int scan_int(InReader *r, int *out_val) {
    return scan_base_int(r, out_val, 10, 0);
}

static int scan_uint(InReader *r, unsigned int *out_val) {
    skip_whitespace(r);
    int c = reader_getc(r);
    if (c == -1) {
        return 0;
    }
    if (c != '+') {
        reader_ungetc(r, c);
    }

    unsigned long long res = 0;
    size_t digits_read = 0;

    while ((c = reader_getc(r)) != -1) {
        if (c >= '0' && c <= '9') {
            digits_read++;
            res = res * 10 + (c - '0');
            if (res > UINT_MAX) {
                res = UINT_MAX;
            }
        } else {
            reader_ungetc(r, c);
            break;
        }
    }

    if (digits_read == 0) {
        return 0;
    }

    *out_val = (unsigned int)res;
    return 1;
}

static int scan_double(InReader *r, double *out_val) {
    skip_whitespace(r);
    int c = reader_getc(r);
    if (c == -1) {
        return 0;
    }

    double sign = 1.0;
    if (c == '-') {
        sign = -1.0;
    } else if (c == '+') {
        sign = 1.0;
    } else {
        reader_ungetc(r, c);
    }

    double integer = 0.0;
    size_t digits_int = 0;
    while ((c = reader_getc(r)) != -1) {
        if (c >= '0' && c <= '9') {
            digits_int++;
            integer = integer * 10.0 + (c - '0');
        } else {
            reader_ungetc(r, c);
            break;
        }
    }

    double frac = 0.0;
    double div = 1.0;
    size_t digits_frac = 0;
    c = reader_getc(r);
    if (c == '.') {
        while ((c = reader_getc(r)) != -1) {
            if (c >= '0' && c <= '9') {
                digits_frac++;
                frac = frac * 10.0 + (c - '0');
                div *= 10.0;
            } else {
                reader_ungetc(r, c);
                break;
            }
        }
    } else if (c != -1) {
        reader_ungetc(r, c);
    }

    if (digits_int == 0 && digits_frac == 0) {
        return 0;
    }

    *out_val = sign * (integer + frac / div);
    return 1;
}

static int scan_string(InReader *r, char *out_str) {
    skip_whitespace(r);
    int c = reader_getc(r);
    if (c == -1) {
        return 0;
    }
    reader_ungetc(r, c);

    size_t len = 0;
    while ((c = reader_getc(r)) != -1) {
        if (!is_whitespace(c)) {
            out_str[len++] = (char)c;
        } else {
            reader_ungetc(r, c);
            break;
        }
    }
    out_str[len] = '\0';
    return (len > 0);
}

static int scan_char(InReader *r, char *out_char) {
    int c = reader_getc(r);
    if (c == -1) {
        return 0;
    }
    *out_char = (char)c;
    return 1;
}

static int over_vscan(InReader *r, const char *format, va_list args) {
    if (format == NULL) {
        return -1;
    }

    int matched_count = 0;
    const char *p = format;

    while (*p != '\0') {
        if (is_whitespace(*p)) {
            skip_whitespace(r);
            while (*p != '\0' && is_whitespace(*p)) {
                p++;
            }
            continue;
        }

        if (*p != '%') {
            int c = reader_getc(r);
            if (c != (unsigned char)*p) {
                if (c != -1) {
                    reader_ungetc(r, c);
                }
                break;
            }
            p++;
            continue;
        }

        p++;
        if (*p == '\0') {
            break;
        }

        if (p[0] == 'R' && p[1] == 'o') {
            int *out = va_arg(args, int *);
            if (!scan_roman(r, out)) {
                break;
            }
            matched_count++;
            p += 2;
        } else if (p[0] == 'Z' && p[1] == 'r') {
            unsigned int *out = va_arg(args, unsigned int *);
            if (!scan_zeckendorf(r, out)) {
                break;
            }
            matched_count++;
            p += 2;
        } else if (p[0] == 'C' && p[1] == 'v') {
            int *out = va_arg(args, int *);
            int base = va_arg(args, int);
            if (!scan_base_int(r, out, base, 0)) {
                break;
            }
            matched_count++;
            p += 2;
        } else if (p[0] == 'C' && p[1] == 'V') {
            int *out = va_arg(args, int *);
            int base = va_arg(args, int);
            if (!scan_base_int(r, out, base, 1)) {
                break;
            }
            matched_count++;
            p += 2;
        } else if (p[0] == '%') {
            int c = reader_getc(r);
            if (c != '%') {
                if (c != -1) {
                    reader_ungetc(r, c);
                }
                break;
            }
            p++;
        } else if (p[0] == 'd' || p[0] == 'i') {
            int *out = va_arg(args, int *);
            if (!scan_int(r, out)) {
                break;
            }
            matched_count++;
            p++;
        } else if (p[0] == 'u') {
            unsigned int *out = va_arg(args, unsigned int *);
            if (!scan_uint(r, out)) {
                break;
            }
            matched_count++;
            p++;
        } else if (p[0] == 'l' && p[1] == 'f') {
            double *out = va_arg(args, double *);
            if (!scan_double(r, out)) {
                break;
            }
            matched_count++;
            p += 2;
        } else if (p[0] == 'f') {
            float *out = va_arg(args, float *);
            double d = 0.0;
            if (!scan_double(r, &d)) {
                break;
            }
            *out = (float)d;
            matched_count++;
            p++;
        } else if (p[0] == 's') {
            char *out = va_arg(args, char *);
            if (!scan_string(r, out)) {
                break;
            }
            matched_count++;
            p++;
        } else if (p[0] == 'c') {
            char *out = va_arg(args, char *);
            if (!scan_char(r, out)) {
                break;
            }
            matched_count++;
            p++;
        } else {
            break;
        }
    }

    return matched_count;
}

int oversscanf(const char *str, const char *format, ...) {
    if (str == NULL || format == NULL) {
        return -1;
    }
    InReader r = {NULL, str, 0};
    va_list args;
    va_start(args, format);
    int res = over_vscan(&r, format, args);
    va_end(args);
    return res;
}

int overfscanf(FILE *stream, const char *format, ...) {
    if (stream == NULL || format == NULL) {
        return -1;
    }
    InReader r = {stream, NULL, 0};
    va_list args;
    va_start(args, format);
    int res = over_vscan(&r, format, args);
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
        res = res * 10 + (*str - '0');
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

void run_demonstration(void) {
    printf("=== ДЕМОНСТРАЦИЯ oversscanf И overfscanf ===\n\n");

    const char *test_input = "MCMXCIV 010011 ff FF 42 hello 3.14";
    printf("Входная строка для oversscanf: \"%s\"\n\n", test_input);

    int roman_val = 0;
    unsigned int zeck_val = 0;
    int base_low = 0;
    int base_up = 0;
    int int_val = 0;
    char str_val[64];
    float float_val = 0.0f;

    int matched = oversscanf(test_input, "%Ro %Zr %Cv %CV %d %s %f",
                             &roman_val,
                             &zeck_val,
                             &base_low, 16,
                             &base_up, 16,
                             &int_val,
                             str_val,
                             &float_val);

    printf("Успешно считано аргументов (matched): %d\n", matched);
    printf("  [%%Ro] Римское число 'MCMXCIV': %d\n", roman_val);
    printf("  [%%Zr] Цекендорфово число '010011': %u\n", zeck_val);
    printf("  [%%Cv] Число 'ff' в base 16: %d\n", base_low);
    printf("  [%%CV] Число 'FF' в base 16: %d\n", base_up);
    printf("  [%%d]  Число '42': %d\n", int_val);
    printf("  [%%s]  Строка: '%s'\n", str_val);
    printf("  [%%f]  Вещественное число: %.2f\n\n", (double)float_val);

    FILE *ftmp = fopen("demo_input.txt", "w");
    if (ftmp != NULL) {
        fprintf(ftmp, "MMXXVI 10100011 1010 777\n");
        fclose(ftmp);

        FILE *fin = fopen("demo_input.txt", "r");
        if (fin != NULL) {
            int f_ro = 0;
            unsigned int f_zr = 0;
            int f_cv = 0;
            int f_d = 0;
            int f_matched = overfscanf(fin, "%Ro %Zr %Cv %d", &f_ro, &f_zr, &f_cv, 2, &f_d);
            fclose(fin);

            printf("Считывание из файла через overfscanf (demo_input.txt):\n");
            printf("  Успешно считано (matched): %d\n", f_matched);
            printf("  [%%Ro] 'MMXXVI' = %d\n", f_ro);
            printf("  [%%Zr] '10100011' = %u\n", f_zr);
            printf("  [%%Cv] '1010' base 2 = %d\n", f_cv);
            printf("  [%%d]  '777' = %d\n", f_d);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1 || (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'd')) {
        run_demonstration();
        return 0;
    }

    const char *flag = argv[1];
    if (flag[0] != '-' && flag[0] != '/') {
        printf("Ошибка: неверный формат флага. Ожидается -<флаг>.\n");
        return 1;
    }

    const char *spec = flag + 1;

    if (spec[0] == 'R' && spec[1] == 'o' && spec[2] == '\0') {
        if (argc != 3) {
            printf("Ошибка: для флага -Ro ожидается 1 аргумент: строка римского числа.\n");
            return 1;
        }
        int val = 0;
        int m = oversscanf(argv[2], "%Ro", &val);
        if (m == 1) {
            printf("Считано (%%Ro): %d\n", val);
        } else {
            printf("Ошибка: не удалось распознать римское число.\n");
            return 1;
        }
    } else if (spec[0] == 'Z' && spec[1] == 'r' && spec[2] == '\0') {
        if (argc != 3) {
            printf("Ошибка: для флага -Zr ожидается 1 аргумент: цекендорфово представление.\n");
            return 1;
        }
        unsigned int val = 0;
        int m = oversscanf(argv[2], "%Zr", &val);
        if (m == 1) {
            printf("Считано (%%Zr): %u\n", val);
        } else {
            printf("Ошибка: не удалось распознать цекендорфово представление.\n");
            return 1;
        }
    } else if (spec[0] == 'C' && (spec[1] == 'v' || spec[1] == 'V') && spec[2] == '\0') {
        if (argc != 4) {
            printf("Ошибка: для флага -%s ожидается 2 аргумента: <строка> <основание [2..36]>.\n", spec);
            return 1;
        }
        int base = 0;
        if (!parse_int(argv[3], &base)) {
            printf("Ошибка: некорректное основание системы счисления.\n");
            return 1;
        }
        int val = 0;
        int m = 0;
        if (spec[1] == 'v') {
            m = oversscanf(argv[2], "%Cv", &val, base);
        } else {
            m = oversscanf(argv[2], "%CV", &val, base);
        }
        if (m == 1) {
            printf("Считано (-%s): %d\n", spec, val);
        } else {
            printf("Ошибка: не удалось считать число в системе счисления %d.\n", base);
            return 1;
        }
    } else if (spec[0] == 'f' && spec[1] == 'i' && spec[2] == 'l' && spec[3] == 'e' && spec[4] == '\0') {
        if (argc != 3) {
            printf("Ошибка: для флага -file требуется путь к файлу.\n");
            return 1;
        }
        FILE *f = fopen(argv[2], "r");
        if (f == NULL) {
            printf("Ошибка: невозможно открыть файл '%s'.\n", argv[2]);
            return 1;
        }
        int ro = 0;
        unsigned int zr = 0;
        int cv = 0;
        int m = overfscanf(f, "%Ro %Zr %Cv", &ro, &zr, &cv, 16);
        fclose(f);
        if (m >= 1) {
            printf("Успешно считано из файла: %d значений (Ro=%d, Zr=%u, Cv=%d)\n", m, ro, zr, cv);
        } else {
            printf("Ошибка: формат данных в файле не распознан.\n");
            return 1;
        }
    } else {
        printf("Ошибка: неизвестный флаг '-%s'.\n", spec);
        return 1;
    }

    return 0;
}