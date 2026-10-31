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

static int compare_double(double a, double b, double eps) {
    double diff = a - b;
    if (fabs(diff) < eps) {
        return 0;
    }
    return (a > b) ? 1 : -1;
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

status_code_t reexpand_polynomial(double eps,
                                  double a,
                                  double **out_coeffs,
                                  int degree,
                                  ...) {
    if (out_coeffs == NULL || eps <= 0.0 || degree < 0) {
        return STATUS_INVALID_ARGUMENT;
    }

    size_t count = (size_t)(degree + 1);
    double *g = (double *)malloc(count * sizeof(double));
    if (g == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    va_list args;
    va_start(args, degree);

    for (size_t i = 0; i < count; ++i) {
        g[i] = va_arg(args, double);
    }
    va_end(args);

    for (size_t k = 0; k < (size_t)degree; ++k) {
        for (size_t i = (size_t)degree; i > k; --i) {
            g[i - 1] += a * g[i];
        }
    }

    for (size_t i = 0; i < count; ++i) {
        if (compare_double(g[i], 0.0, eps) == 0) {
            g[i] = 0.0;
        }
    }

    *out_coeffs = g;
    return STATUS_OK;
}

void print_polynomial_result(const double *orig_f, const double *new_g, int degree, double a) {
    printf("========================================================================\n");
    printf("Исходный многочлен f(x):\n  f(x) = ");
    int first = 1;
    for (int i = 0; i <= degree; ++i) {
        double c = orig_f[i];
        if (fabs(c) < 1e-12 && degree > 0) {
            continue;
        }
        if (first) {
            if (i == 0) {
                printf("%.4f", c);
            } else if (i == 1) {
                printf("%.4f*x", c);
            } else {
                printf("%.4f*x^%d", c, i);
            }
            first = 0;
        } else {
            if (c >= 0) {
                printf(" + %.4f", c);
            } else {
                printf(" - %.4f", -c);
            }
            if (i == 1) {
                printf("*x");
            } else if (i > 1) {
                printf("*x^%d", i);
            }
        }
    }
    if (first) {
        printf("0.0000");
    }
    printf("\n\n");

    printf("Переразложение по степеням (x - a) при a = %.4f:\n  f(x) = ", a);
    first = 1;
    for (int i = 0; i <= degree; ++i) {
        double c = new_g[i];
        if (fabs(c) < 1e-12 && degree > 0) {
            continue;
        }
        if (first) {
            if (i == 0) {
                printf("%.4f", c);
            } else if (i == 1) {
                if (fabs(a) < 1e-12) {
                    printf("%.4f*x", c);
                } else if (a > 0) {
                    printf("%.4f*(x - %.4f)", c, a);
                } else {
                    printf("%.4f*(x + %.4f)", c, -a);
                }
            } else {
                if (fabs(a) < 1e-12) {
                    printf("%.4f*x^%d", c, i);
                } else if (a > 0) {
                    printf("%.4f*(x - %.4f)^%d", c, a, i);
                } else {
                    printf("%.4f*(x + %.4f)^%d", c, -a, i);
                }
            }
            first = 0;
        } else {
            if (c >= 0) {
                printf(" + ");
            } else {
                printf(" - ");
                c = -c;
            }
            if (i == 1) {
                if (fabs(a) < 1e-12) {
                    printf("%.4f*x", c);
                } else if (a > 0) {
                    printf("%.4f*(x - %.4f)", c, a);
                } else {
                    printf("%.4f*(x + %.4f)", c, -a);
                }
            } else {
                if (fabs(a) < 1e-12) {
                    printf("%.4f*x^%d", c, i);
                } else if (a > 0) {
                    printf("%.4f*(x - %.4f)^%d", c, a, i);
                } else {
                    printf("%.4f*(x + %.4f)^%d", c, -a, i);
                }
            }
        }
    }
    if (first) {
        printf("0.0000");
    }
    printf("\n\nКоэффициенты g_k (по возрастанию степеней):\n");
    for (int i = 0; i <= degree; ++i) {
        printf("  g_%d = %10.6f\n", i, new_g[i]);
    }
    printf("========================================================================\n");
}

void run_demonstration(void) {
    printf("=== ДЕМОНСТРАЦИЯ ЗАДАНИЯ 10 (ПЕРЕРАЗЛОЖЕНИЕ МНОГОЧЛЕНА) ===\n\n");
    double *g = NULL;
    double eps = 1e-9;

    printf("Пример 1: f(x) = 1 + 2x + 3x^2, точка a = 1.0\n");
    double f1[3] = {1.0, 2.0, 3.0};
    if (reexpand_polynomial(eps, 1.0, &g, 2, f1[0], f1[1], f1[2]) == STATUS_OK) {
        print_polynomial_result(f1, g, 2, 1.0);
        free(g);
    }
    printf("\n");

    printf("Пример 2: f(x) = -2 + x - 3x^2 + x^3, точка a = 2.0\n");
    double f2[4] = {-2.0, 1.0, -3.0, 1.0};
    if (reexpand_polynomial(eps, 2.0, &g, 3, f2[0], f2[1], f2[2], f2[3]) == STATUS_OK) {
        print_polynomial_result(f2, g, 3, 2.0);
        free(g);
    }
    printf("\n");

    printf("Пример 3: f(x) = 42 (степень 0), точка a = -5.0\n");
    double f3[1] = {42.0};
    if (reexpand_polynomial(eps, -5.0, &g, 0, f3[0]) == STATUS_OK) {
        print_polynomial_result(f3, g, 0, -5.0);
        free(g);
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
        printf("Использование: %s <точность_eps> <значение_a> <степень_n> <f_0> [f_1 ... f_n]\n", argv[0]);
        return 1;
    }

    double eps = 0.0;
    if (!parse_double(argv[1], &eps) || eps <= 0.0) {
        printf("Ошибка: точность epsilon должна быть положительным вещественным числом.\n");
        return 1;
    }

    double a = 0.0;
    if (!parse_double(argv[2], &a)) {
        printf("Ошибка: значение 'a' должно быть вещественным числом.\n");
        return 1;
    }

    int degree = 0;
    if (!parse_int(argv[3], &degree) || degree < 0) {
        printf("Ошибка: степень многочлена должна быть целым неотрицательным числом.\n");
        return 1;
    }

    if (degree > 10) {
        printf("Ошибка: поддерживается степень многочлена не более 10.\n");
        return 1;
    }

    int expected_argc = degree + 5;
    if (argc != expected_argc) {
        printf("Ошибка: неверное количество коэффициентов. Для степени %d ожидалось ровно %d коэффициентов f_0 ... f_%d, а передано %d.\n",
               degree, degree + 1, degree, argc - 4);
        return 1;
    }

    double f[11];
    for (int i = 0; i <= degree; ++i) {
        if (!parse_double(argv[4 + i], &f[i])) {
            printf("Ошибка: некорректный коэффициент f_%d ('%s'). Ожидалось вещественное число.\n", i, argv[4 + i]);
            return 1;
        }
    }

    double *g = NULL;
    status_code_t st = STATUS_OK;

    switch (degree) {
        case 0:
            st = reexpand_polynomial(eps, a, &g, 0, f[0]);
            break;
        case 1:
            st = reexpand_polynomial(eps, a, &g, 1, f[0], f[1]);
            break;
        case 2:
            st = reexpand_polynomial(eps, a, &g, 2, f[0], f[1], f[2]);
            break;
        case 3:
            st = reexpand_polynomial(eps, a, &g, 3, f[0], f[1], f[2], f[3]);
            break;
        case 4:
            st = reexpand_polynomial(eps, a, &g, 4, f[0], f[1], f[2], f[3], f[4]);
            break;
        case 5:
            st = reexpand_polynomial(eps, a, &g, 5, f[0], f[1], f[2], f[3], f[4], f[5]);
            break;
        case 6:
            st = reexpand_polynomial(eps, a, &g, 6, f[0], f[1], f[2], f[3], f[4], f[5], f[6]);
            break;
        case 7:
            st = reexpand_polynomial(eps, a, &g, 7, f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7]);
            break;
        case 8:
            st = reexpand_polynomial(eps, a, &g, 8, f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]);
            break;
        case 9:
            st = reexpand_polynomial(eps, a, &g, 9, f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8], f[9]);
            break;
        case 10:
            st = reexpand_polynomial(eps, a, &g, 10, f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8], f[9], f[10]);
            break;
        default:
            st = STATUS_INVALID_ARGUMENT;
            break;
    }

    if (st == STATUS_INVALID_ARGUMENT) {
        printf("Ошибка: переданы некорректные аргументы.\n");
        return 1;
    } else if (st == STATUS_MEMORY_ERROR) {
        printf("Ошибка: не удалось выделить память.\n");
        return 1;
    } else if (st != STATUS_OK) {
        printf("Ошибка при переразложении многочлена.\n");
        return 1;
    }

    print_polynomial_result(f, g, degree, a);
    free(g);

    return 0;
}