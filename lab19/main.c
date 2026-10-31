#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_MEMORY_ERROR
} status_code_t;

typedef struct {
    double value;
    int64_t numerator;
    int64_t denominator;
    int has_finite_repr;
} FractionCheckResult;

static int64_t gcd(int64_t a, int64_t b) {
    if (a < 0) {
        a = -a;
    }
    if (b < 0) {
        b = -b;
    }
    while (b != 0) {
        int64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

static int compare_double(double a, double b, double eps) {
    double diff = a - b;
    if (fabs(diff) < eps) {
        return 0;
    }
    return (a > b) ? 1 : -1;
}

int double_to_fraction(double x, double eps, int64_t *out_p, int64_t *out_q) {
    if (x <= 0.0 || x >= 1.0 || eps <= 0.0 || out_p == NULL || out_q == NULL) {
        return 0;
    }

    int64_t p0 = 0, q0 = 1;
    int64_t p1 = 1, q1 = 0;
    int64_t p = 0, q = 1;

    double cur = x;
    int64_t a = (int64_t)cur;
    double rem = cur - a;

    p = a * p1 + p0;
    q = a * q1 + q0;

    int max_iter = 64;
    while (fabs(x - (double)p / (double)q) > eps && max_iter-- > 0) {
        if (fabs(rem) < 1e-15) {
            break;
        }
        double inv = 1.0 / rem;
        a = (int64_t)(inv + 1e-11);
        rem = inv - a;

        int64_t p_next = a * p + p1;
        int64_t q_next = a * q + q1;

        if (q_next <= 0) {
            break;
        }

        p1 = p;
        q1 = q;
        p = p_next;
        q = q_next;
    }

    int64_t g = gcd(p, q);
    if (g == 0) {
        return 0;
    }
    *out_p = p / g;
    *out_q = q / g;
    return 1;
}

int check_finite_representation(int64_t q, int base) {
    if (q <= 0 || base < 2) {
        return 0;
    }
    int64_t cur_q = q;
    while (1) {
        int64_t g = gcd(cur_q, (int64_t)base);
        if (g == 1) {
            break;
        }
        cur_q /= g;
    }
    return (cur_q == 1);
}

status_code_t check_fractions_finite(FractionCheckResult **out_results,
                                     double eps,
                                     int base,
                                     size_t count,
                                     ...) {
    if (out_results == NULL || eps <= 0.0 || base < 2 || base > 36 || count == 0) {
        return STATUS_INVALID_ARGUMENT;
    }

    va_list args;
    va_start(args, count);

    double *vals = (double *)malloc(count * sizeof(double));
    if (vals == NULL) {
        va_end(args);
        return STATUS_MEMORY_ERROR;
    }

    for (size_t i = 0; i < count; ++i) {
        vals[i] = va_arg(args, double);
        if (compare_double(vals[i], 0.0, eps) <= 0 || compare_double(vals[i], 1.0, eps) >= 0) {
            free(vals);
            va_end(args);
            return STATUS_INVALID_ARGUMENT;
        }
    }
    va_end(args);

    FractionCheckResult *results = (FractionCheckResult *)malloc(count * sizeof(FractionCheckResult));
    if (results == NULL) {
        free(vals);
        return STATUS_MEMORY_ERROR;
    }

    for (size_t i = 0; i < count; ++i) {
        results[i].value = vals[i];
        int64_t p = 0, q = 0;
        if (!double_to_fraction(vals[i], eps, &p, &q)) {
            free(vals);
            free(results);
            return STATUS_INVALID_ARGUMENT;
        }
        results[i].numerator = p;
        results[i].denominator = q;
        results[i].has_finite_repr = check_finite_representation(q, base);
    }

    free(vals);
    *out_results = results;
    return STATUS_OK;
}

void print_fraction_results(const FractionCheckResult *results, size_t count, int base) {
    if (results == NULL) {
        return;
    }
    printf("========================================================================\n");
    printf("Основание системы счисления: %d\n", base);
    printf("Проверено дробей: %zu\n", count);
    printf("------------------------------------------------------------------------\n");
    for (size_t i = 0; i < count; ++i) {
        printf("  [%zu] Дробь: %-10g (Несократимый вид: %lld/%lld) -> %s\n",
               i + 1,
               results[i].value,
               (long long)results[i].numerator,
               (long long)results[i].denominator,
               results[i].has_finite_repr ? "ИМЕЕТ конечное представление" : "НЕ имеет конечного представления (бесконечная/периодическая)");
    }
    printf("========================================================================\n");
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
    printf("=== ДЕМОНСТРАЦИЯ ЗАДАНИЯ 9 (КОНЕЧНОЕ ПРЕДСТАВЛЕНИЕ ДРОБЕЙ) ===\n\n");
    FractionCheckResult *results = NULL;

    printf("1. Проверка в двоичной системе (base 2):\n");
    if (check_fractions_finite(&results, 1e-9, 2, 4, 0.5, 0.25, 0.2, 0.375) == STATUS_OK) {
        print_fraction_results(results, 4, 2);
        free(results);
    }
    printf("\n");

    printf("2. Проверка в десятичной системе (base 10):\n");
    if (check_fractions_finite(&results, 1e-9, 10, 3, 0.2, 0.05, 0.0016) == STATUS_OK) {
        print_fraction_results(results, 3, 10);
        free(results);
    }
    printf("\n");

    printf("3. Проверка в троичной системе (base 3):\n");
    if (check_fractions_finite(&results, 1e-9, 3, 2, 0.5, 1.0 / 3.0) == STATUS_OK) {
        print_fraction_results(results, 2, 3);
        free(results);
    }
    printf("\n");

    printf("4. Проверка в шестеричной системе (base 6, простые делители 2 и 3):\n");
    if (check_fractions_finite(&results, 1e-9, 6, 3, 0.5, 1.0 / 3.0, 0.2) == STATUS_OK) {
        print_fraction_results(results, 3, 6);
        free(results);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc == 1 || (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'd')) {
        run_demonstration();
        return 0;
    }

    if (argc < 4) {
        printf("Ошибка: недостаточно аргументов.\n");
        printf("Использование: %s <основание [2..36]> <точность_eps> <дробь1> [дробь2 ...]\n", argv[0]);
        return 1;
    }

    int base = 0;
    if (!parse_int(argv[1], &base)) {
        printf("Ошибка: некорректное основание системы счисления.\n");
        return 1;
    }

    if (base < 2 || base > 36) {
        printf("Ошибка: основание системы счисления должно быть в диапазоне [2..36].\n");
        return 1;
    }

    double eps = 0.0;
    if (!parse_double(argv[2], &eps) || eps <= 0.0) {
        printf("Ошибка: точность epsilon должна быть положительным вещественным числом.\n");
        return 1;
    }

    size_t count = (size_t)(argc - 3);
    double *vals = (double *)malloc(count * sizeof(double));
    if (vals == NULL) {
        printf("Ошибка: не удалось выделить память.\n");
        return 1;
    }

    for (size_t i = 0; i < count; ++i) {
        double v = 0.0;
        if (!parse_double(argv[3 + i], &v)) {
            printf("Ошибка: передан некорректный аргумент дроби '%s'.\n", argv[3 + i]);
            free(vals);
            return 1;
        }
        if (compare_double(v, 0.0, eps) <= 0 || compare_double(v, 1.0, eps) >= 0) {
            printf("Ошибка: значение дроби '%s' выходит за пределы допустимого диапазона (0; 1).\n", argv[3 + i]);
            free(vals);
            return 1;
        }
        vals[i] = v;
    }

    FractionCheckResult *results = NULL;
    status_code_t st = STATUS_OK;

    switch (count) {
        case 1:
            st = check_fractions_finite(&results, eps, base, 1, vals[0]);
            break;
        case 2:
            st = check_fractions_finite(&results, eps, base, 2, vals[0], vals[1]);
            break;
        case 3:
            st = check_fractions_finite(&results, eps, base, 3, vals[0], vals[1], vals[2]);
            break;
        case 4:
            st = check_fractions_finite(&results, eps, base, 4, vals[0], vals[1], vals[2], vals[3]);
            break;
        case 5:
            st = check_fractions_finite(&results, eps, base, 5, vals[0], vals[1], vals[2], vals[3], vals[4]);
            break;
        case 6:
            st = check_fractions_finite(&results, eps, base, 6, vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
            break;
        case 7:
            st = check_fractions_finite(&results, eps, base, 7, vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6]);
            break;
        case 8:
            st = check_fractions_finite(&results, eps, base, 8, vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7]);
            break;
        case 9:
            st = check_fractions_finite(&results, eps, base, 9, vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7], vals[8]);
            break;
        case 10:
            st = check_fractions_finite(&results, eps, base, 10, vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7], vals[8], vals[9]);
            break;
        default:
            printf("Ошибка: поддерживается одновременная проверка до 10 дробей через аргументы командной строки.\n");
            free(vals);
            return 1;
    }

    free(vals);

    if (st == STATUS_INVALID_ARGUMENT) {
        printf("Ошибка: переданы некорректные аргументы.\n");
        return 1;
    } else if (st == STATUS_MEMORY_ERROR) {
        printf("Ошибка: не удалось выделить память.\n");
        return 1;
    } else if (st != STATUS_OK) {
        printf("Ошибка при проверке дробей.\n");
        return 1;
    }

    print_fraction_results(results, count, base);
    free(results);
    return 0;
}