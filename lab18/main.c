#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_MEMORY_ERROR,
    STATUS_FORMAT_ERROR
} status_code_t;

static size_t my_strlen(const char *s) {
    size_t l = 0;
    while (s && s[l] != '\0') {
        l++;
    }
    return l;
}

static char *my_strdup(const char *s) {
    if (s == NULL) {
        return NULL;
    }
    size_t len = my_strlen(s);
    char *res = (char *)malloc((len + 1) * sizeof(char));
    if (res == NULL) {
        return NULL;
    }
    for (size_t i = 0; i <= len; ++i) {
        res[i] = s[i];
    }
    return res;
}

static int char_to_digit(char c, int base) {
    int val = -1;
    if (c >= '0' && c <= '9') {
        val = c - '0';
    } else if (c >= 'a' && c <= 'z') {
        val = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'Z') {
        val = c - 'A' + 10;
    }
    if (val >= 0 && val < base) {
        return val;
    }
    return -1;
}

static char digit_to_char(int d) {
    if (d >= 0 && d <= 9) {
        return (char)('0' + d);
    }
    if (d >= 10 && d <= 35) {
        return (char)('A' + d - 10);
    }
    return '?';
}

static int validate_base_number(const char *str, int base) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    if (base < 2 || base > 36) {
        return 0;
    }
    const char *p = str;
    if (*p == '+') {
        p++;
    }
    if (*p == '\0') {
        return 0;
    }
    while (*p != '\0') {
        if (char_to_digit(*p, base) == -1) {
            return 0;
        }
        p++;
    }
    return 1;
}

status_code_t add_column(const char *num1, const char *num2, int base, char **out_result) {
    if (num1 == NULL || num2 == NULL || out_result == NULL || base < 2 || base > 36) {
        return STATUS_INVALID_ARGUMENT;
    }
    if (!validate_base_number(num1, base) || !validate_base_number(num2, base)) {
        return STATUS_FORMAT_ERROR;
    }

    const char *p1 = (*num1 == '+') ? num1 + 1 : num1;
    const char *p2 = (*num2 == '+') ? num2 + 1 : num2;

    size_t len1 = my_strlen(p1);
    size_t len2 = my_strlen(p2);
    size_t max_len = (len1 > len2 ? len1 : len2);

    char *res = (char *)malloc((max_len + 2) * sizeof(char));
    if (res == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    size_t i = len1;
    size_t j = len2;
    int carry = 0;
    size_t out_idx = 0;

    while (i > 0 || j > 0 || carry > 0) {
        int d1 = (i > 0) ? char_to_digit(p1[--i], base) : 0;
        int d2 = (j > 0) ? char_to_digit(p2[--j], base) : 0;
        int sum = d1 + d2 + carry;
        carry = sum / base;
        res[out_idx++] = digit_to_char(sum % base);
    }

    for (size_t l = 0; l < out_idx / 2; ++l) {
        char tmp = res[l];
        res[l] = res[out_idx - 1 - l];
        res[out_idx - 1 - l] = tmp;
    }
    res[out_idx] = '\0';

    size_t lead_zeros = 0;
    while (lead_zeros + 1 < out_idx && res[lead_zeros] == '0') {
        lead_zeros++;
    }

    if (lead_zeros > 0) {
        size_t new_len = out_idx - lead_zeros;
        for (size_t k = 0; k <= new_len; ++k) {
            res[k] = res[lead_zeros + k];
        }
    }

    *out_result = res;
    return STATUS_OK;
}

status_code_t sum_numbers_in_base(char **out_result, int base, size_t count, ...) {
    if (out_result == NULL || base < 2 || base > 36 || count == 0) {
        return STATUS_INVALID_ARGUMENT;
    }

    va_list args;
    va_start(args, count);

    char *current_sum = my_strdup("0");
    if (current_sum == NULL) {
        va_end(args);
        return STATUS_MEMORY_ERROR;
    }

    for (size_t i = 0; i < count; ++i) {
        const char *next_num = va_arg(args, const char *);
        if (next_num == NULL) {
            free(current_sum);
            va_end(args);
            return STATUS_INVALID_ARGUMENT;
        }

        char *next_sum = NULL;
        status_code_t st = add_column(current_sum, next_num, base, &next_sum);
        free(current_sum);
        if (st != STATUS_OK) {
            va_end(args);
            return st;
        }
        current_sum = next_sum;
    }

    va_end(args);
    *out_result = current_sum;
    return STATUS_OK;
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
        if (*str < '0' || *str > '9') {
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
    printf("=== ДЕМОНСТРАЦИЯ ЗАДАНИЯ 8 (СЛОЖЕНИЕ В СИСТЕМАХ СЧИСЛЕНИЯ) ===\n\n");
    char *res = NULL;

    sum_numbers_in_base(&res, 2, 4, "1011", "1101", "1", "0");
    printf("1. Двоичная система (base 2):\n");
    printf("   1011_2 + 1101_2 + 1_2 + 0_2 = %s_2  (11 + 13 + 1 + 0 = 25)\n\n", res);
    free(res);

    sum_numbers_in_base(&res, 16, 3, "1A", "02B", "C");
    printf("2. Шестнадцатеричная система (base 16):\n");
    printf("   1A_16 + 02B_16 + C_16 = %s_16  (26 + 43 + 12 = 81)\n\n", res);
    free(res);

    sum_numbers_in_base(&res, 36, 2, "ZZ", "1");
    printf("3. Система по основанию 36 (base 36):\n");
    printf("   ZZ_36 + 1_36 = %s_36  (1295 + 1 = 1296 = 36^2)\n\n", res);
    free(res);

    sum_numbers_in_base(&res, 10, 2, "99999999999999999999999999999999999999999999999999", "1");
    printf("4. Длинные числа (50 девяток + 1):\n");
    printf("   99...99_10 + 1_10 = %s_10\n\n", res);
    free(res);

    sum_numbers_in_base(&res, 10, 3, "00000", "00", "0");
    printf("5. Ведущие нули (00000 + 00 + 0):\n");
    printf("   Результат: %s (без лишних нулей)\n\n", res);
    free(res);
}

int main(int argc, char *argv[]) {
    if (argc == 1 || (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'd')) {
        run_demonstration();
        return 0;
    }

    if (argc < 3) {
        printf("Ошибка: недостаточно аргументов.\n");
        printf("Использование: %s <основание [2..36]> <число1> [число2 ...]\n", argv[0]);
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

    size_t count = (size_t)(argc - 2);
    char *res = NULL;
    status_code_t st = STATUS_OK;

    switch (count) {
        case 1:
            st = sum_numbers_in_base(&res, base, 1, argv[2]);
            break;
        case 2:
            st = sum_numbers_in_base(&res, base, 2, argv[2], argv[3]);
            break;
        case 3:
            st = sum_numbers_in_base(&res, base, 3, argv[2], argv[3], argv[4]);
            break;
        case 4:
            st = sum_numbers_in_base(&res, base, 4, argv[2], argv[3], argv[4], argv[5]);
            break;
        case 5:
            st = sum_numbers_in_base(&res, base, 5, argv[2], argv[3], argv[4], argv[5], argv[6]);
            break;
        case 6:
            st = sum_numbers_in_base(&res, base, 6, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
            break;
        case 7:
            st = sum_numbers_in_base(&res, base, 7, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8]);
            break;
        case 8:
            st = sum_numbers_in_base(&res, base, 8, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9]);
            break;
        case 9:
            st = sum_numbers_in_base(&res, base, 9, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10]);
            break;
        case 10:
            st = sum_numbers_in_base(&res, base, 10, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], argv[11]);
            break;
        default:
            printf("Ошибка: поддерживается сложение до 10 чисел через аргументы командной строки.\n");
            return 1;
    }

    if (st == STATUS_FORMAT_ERROR) {
        printf("Ошибка: число содержит символы, недопустимые для системы счисления по основанию %d.\n", base);
        return 1;
    } else if (st == STATUS_MEMORY_ERROR) {
        printf("Ошибка: не удалось выделить память.\n");
        return 1;
    } else if (st == STATUS_INVALID_ARGUMENT) {
        printf("Ошибка: переданы некорректные аргументы.\n");
        return 1;
    } else if (st != STATUS_OK) {
        printf("Ошибка при вычислении суммы.\n");
        return 1;
    }

    printf("Сумма в системе по основанию %d: %s\n", base, res);
    free(res);
    return 0;
}