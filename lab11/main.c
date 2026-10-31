#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_MEMORY_ERROR
} status_code_t;

static size_t my_strlen(const char *str) {
    size_t len = 0;
    if (str == NULL) {
        return 0;
    }
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

static int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static char to_upper(char c) {
    if (c >= 'a' && c <= 'z') {
        return (char)(c - 'a' + 'A');
    }
    return c;
}

static int parse_unsigned_int(const char *str, unsigned int *out_val) {
    if (str == NULL || out_val == NULL || *str == '\0') {
        return 0;
    }
    if (*str == '+') {
        str++;
    }
    if (*str == '\0') {
        return 0;
    }
    unsigned int res = 0;
    while (*str != '\0') {
        if (!is_digit(*str)) {
            return 0;
        }
        unsigned int digit = (unsigned int)(*str - '0');
        if (res > (UINT_MAX - digit) / 10) {
            return 0;
        }
        res = res * 10 + digit;
        str++;
    }
    *out_val = res;
    return 1;
}

status_code_t task_length(const char *str, size_t *out_length) {
    if (str == NULL || out_length == NULL) {
        return STATUS_INVALID_ARGUMENT;
    }
    *out_length = my_strlen(str);
    return STATUS_OK;
}

status_code_t task_reverse(const char *str, char **out_result) {
    if (str == NULL || out_result == NULL) {
        return STATUS_INVALID_ARGUMENT;
    }
    size_t len = my_strlen(str);
    char *res = (char *)malloc((len + 1) * sizeof(char));
    if (res == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    for (size_t i = 0; i < len; ++i) {
        res[i] = str[len - 1 - i];
    }
    res[len] = '\0';
    *out_result = res;
    return STATUS_OK;
}

status_code_t task_uppercase_odd(const char *str, char **out_result) {
    if (str == NULL || out_result == NULL) {
        return STATUS_INVALID_ARGUMENT;
    }
    size_t len = my_strlen(str);
    char *res = (char *)malloc((len + 1) * sizeof(char));
    if (res == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    for (size_t i = 0; i < len; ++i) {
        if (i % 2 != 0) {
            res[i] = to_upper(str[i]);
        } else {
            res[i] = str[i];
        }
    }
    res[len] = '\0';
    *out_result = res;
    return STATUS_OK;
}

status_code_t task_digits_letters_other(const char *str, char **out_result) {
    if (str == NULL || out_result == NULL) {
        return STATUS_INVALID_ARGUMENT;
    }
    size_t len = my_strlen(str);
    char *res = (char *)malloc((len + 1) * sizeof(char));
    if (res == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    size_t idx = 0;
    for (size_t i = 0; i < len; ++i) {
        if (is_digit(str[i])) {
            res[idx++] = str[i];
        }
    }
    for (size_t i = 0; i < len; ++i) {
        if (is_alpha(str[i])) {
            res[idx++] = str[i];
        }
    }
    for (size_t i = 0; i < len; ++i) {
        if (!is_digit(str[i]) && !is_alpha(str[i])) {
            res[idx++] = str[i];
        }
    }
    res[idx] = '\0';
    *out_result = res;
    return STATUS_OK;
}

status_code_t task_concatenate_random(const char **strings, size_t count, unsigned int seed, char **out_result) {
    if (strings == NULL || count == 0 || out_result == NULL) {
        return STATUS_INVALID_ARGUMENT;
    }
    for (size_t i = 0; i < count; ++i) {
        if (strings[i] == NULL) {
            return STATUS_INVALID_ARGUMENT;
        }
    }

    const char **shuffled = (const char **)malloc(count * sizeof(const char *));
    if (shuffled == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    for (size_t i = 0; i < count; ++i) {
        shuffled[i] = strings[i];
    }

    srand(seed);
    for (size_t i = count; i > 1; --i) {
        size_t j = (size_t)rand() % i;
        const char *temp = shuffled[i - 1];
        shuffled[i - 1] = shuffled[j];
        shuffled[j] = temp;
    }

    size_t total_len = 0;
    for (size_t i = 0; i < count; ++i) {
        total_len += my_strlen(shuffled[i]);
    }

    char *res = (char *)malloc((total_len + 1) * sizeof(char));
    if (res == NULL) {
        free(shuffled);
        return STATUS_MEMORY_ERROR;
    }

    size_t cur = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t slen = my_strlen(shuffled[i]);
        for (size_t k = 0; k < slen; ++k) {
            res[cur++] = shuffled[i][k];
        }
    }
    res[cur] = '\0';
    free(shuffled);

    *out_result = res;
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Ошибка: недостаточно аргументов командной строки.\n");
        return 1;
    }

    const char *flag_arg = argv[1];
    if ((flag_arg[0] != '-' && flag_arg[0] != '/') || flag_arg[1] == '\0' || flag_arg[2] != '\0') {
        printf("Ошибка: некорректный формат флага. Ожидается -флаг или /флаг.\n");
        return 1;
    }

    char flag = flag_arg[1];
    const char *input_str = argv[2];

    if (flag == 'l') {
        if (argc != 3) {
            printf("Ошибка: для флага -l ожидается ровно 2 аргумента: флаг и строка.\n");
            return 1;
        }
        size_t length = 0;
        status_code_t status = task_length(input_str, &length);
        if (status == STATUS_OK) {
            printf("Длина строки: %zu\n", length);
        } else {
            printf("Ошибка при вычислении длины.\n");
            return 1;
        }
    } else if (flag == 'r') {
        if (argc != 3) {
            printf("Ошибка: для флага -r ожидается ровно 2 аргумента: флаг и строка.\n");
            return 1;
        }
        char *result = NULL;
        status_code_t status = task_reverse(input_str, &result);
        if (status == STATUS_OK) {
            printf("Перевернутая строка: %s\n", result);
            free(result);
        } else if (status == STATUS_MEMORY_ERROR) {
            printf("Ошибка: не удалось выделить память.\n");
            return 1;
        } else {
            printf("Ошибка обработки аргументов.\n");
            return 1;
        }
    } else if (flag == 'u') {
        if (argc != 3) {
            printf("Ошибка: для флага -u ожидается ровно 2 аргумента: флаг и строка.\n");
            return 1;
        }
        char *result = NULL;
        status_code_t status = task_uppercase_odd(input_str, &result);
        if (status == STATUS_OK) {
            printf("Результат преобразования нечетных позиций: %s\n", result);
            free(result);
        } else if (status == STATUS_MEMORY_ERROR) {
            printf("Ошибка: не удалось выделить память.\n");
            return 1;
        } else {
            printf("Ошибка обработки аргументов.\n");
            return 1;
        }
    } else if (flag == 'n') {
        if (argc != 3) {
            printf("Ошибка: для флага -n ожидается ровно 2 аргумента: флаг и строка.\n");
            return 1;
        }
        char *result = NULL;
        status_code_t status = task_digits_letters_other(input_str, &result);
        if (status == STATUS_OK) {
            printf("Перегруппированная строка (цифры, буквы, прочие): %s\n", result);
            free(result);
        } else if (status == STATUS_MEMORY_ERROR) {
            printf("Ошибка: не удалось выделить память.\n");
            return 1;
        } else {
            printf("Ошибка обработки аргументов.\n");
            return 1;
        }
    } else if (flag == 'c') {
        if (argc < 4) {
            printf("Ошибка: для флага -c требуется указать строку, seed и далее строки.\n");
            return 1;
        }
        unsigned int seed = 0;
        if (!parse_unsigned_int(argv[3], &seed)) {
            printf("Ошибка: передан некорректный seed (ожидалось неотрицательное целое число unsigned int без переполнения).\n");
            return 1;
        }
        size_t count = (size_t)(argc - 3);
        const char **strings = (const char **)malloc(count * sizeof(const char *));
        if (strings == NULL) {
            printf("Ошибка: не удалось выделить память.\n");
            return 1;
        }
        strings[0] = argv[2];
        for (int i = 4; i < argc; ++i) {
            strings[i - 3] = argv[i];
        }

        char *result = NULL;
        status_code_t status = task_concatenate_random(strings, count, seed, &result);
        free(strings);

        if (status == STATUS_OK) {
            printf("Результат конкатенации: %s\n", result);
            free(result);
        } else if (status == STATUS_MEMORY_ERROR) {
            printf("Ошибка: не удалось выделить память.\n");
            return 1;
        } else {
            printf("Ошибка при конкатенации строк.\n");
            return 1;
        }
    } else {
        printf("Ошибка: неизвестный флаг '-%c'. Допустимые флаги: -l, -r, -u, -n, -c.\n", flag);
        return 1;
    }

    return 0;
}