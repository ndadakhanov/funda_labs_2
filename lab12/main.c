#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_FILE_OPEN_ERROR,
    STATUS_FILE_FORMAT_ERROR,
    STATUS_MEMORY_ERROR
} status_code_t;

typedef struct {
    unsigned int id;
    char *name;
    char *surname;
    double salary;
} Employee;

static int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

static int is_latin_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int is_space(char c) {
    return (c == ' ' || c == '\t');
}

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

static char *my_strdup(const char *src) {
    if (src == NULL) {
        return NULL;
    }
    size_t len = my_strlen(src);
    char *dest = (char *)malloc((len + 1) * sizeof(char));
    if (dest == NULL) {
        return NULL;
    }
    for (size_t i = 0; i <= len; ++i) {
        dest[i] = src[i];
    }
    return dest;
}

static int my_strcmp(const char *s1, const char *s2) {
    if (s1 == NULL || s2 == NULL) {
        return 0;
    }
    while (*s1 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

static int validate_latin_string(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (!is_latin_alpha(str[i])) {
            return 0;
        }
    }
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

static int parse_nonnegative_double(const char *str, double *out_val) {
    if (str == NULL || out_val == NULL || *str == '\0') {
        return 0;
    }
    if (*str == '+') {
        str++;
    }
    if (*str == '\0') {
        return 0;
    }

    int dot_count = 0;
    int digits_before_dot = 0;
    int digits_after_dot = 0;
    const char *p = str;

    while (*p != '\0') {
        if (*p == '.') {
            dot_count++;
            if (dot_count > 1) {
                return 0;
            }
        } else if (is_digit(*p)) {
            if (dot_count == 0) {
                digits_before_dot++;
            } else {
                digits_after_dot++;
            }
        } else {
            return 0;
        }
        p++;
    }

    if (digits_before_dot == 0 && digits_after_dot == 0) {
        return 0;
    }

    double int_part = 0.0;
    p = str;
    while (*p != '\0' && *p != '.') {
        int_part = int_part * 10.0 + (*p - '0');
        if (int_part > 1e14) {
            return 0;
        }
        p++;
    }

    double frac_part = 0.0;
    double divisor = 1.0;
    if (*p == '.') {
        p++;
        while (*p != '\0') {
            frac_part = frac_part * 10.0 + (*p - '0');
            divisor *= 10.0;
            if (divisor > 1e14) {
                return 0;
            }
            p++;
        }
    }

    *out_val = int_part + (frac_part / divisor);
    return 1;
}

static status_code_t read_dynamic_line(FILE *f, char **out_line) {
    size_t capacity = 64;
    size_t length = 0;
    char *buffer = (char *)malloc(capacity * sizeof(char));
    if (buffer == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    int c;
    while ((c = fgetc(f)) != EOF && c != '\n') {
        if (c == '\r') {
            continue;
        }
        if (length + 1 >= capacity) {
            size_t new_cap = capacity * 2;
            char *new_buf = (char *)realloc(buffer, new_cap * sizeof(char));
            if (new_buf == NULL) {
                free(buffer);
                return STATUS_MEMORY_ERROR;
            }
            buffer = new_buf;
            capacity = new_cap;
        }
        buffer[length++] = (char)c;
    }

    if (c == EOF && length == 0) {
        free(buffer);
        *out_line = NULL;
        return STATUS_OK;
    }

    buffer[length] = '\0';
    *out_line = buffer;
    return STATUS_OK;
}

static void free_employees(Employee *arr, size_t count) {
    if (arr == NULL) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        free(arr[i].name);
        free(arr[i].surname);
    }
    free(arr);
}

static status_code_t parse_employee_line(char *line, Employee *emp) {
    char *tokens[5];
    int token_count = 0;
    char *p = line;

    while (*p != '\0') {
        while (*p != '\0' && is_space(*p)) {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        if (token_count < 4) {
            tokens[token_count++] = p;
        } else {
            return STATUS_FILE_FORMAT_ERROR;
        }
        while (*p != '\0' && !is_space(*p)) {
            p++;
        }
        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }

    if (token_count == 0) {
        return STATUS_OK;
    }
    if (token_count != 4) {
        return STATUS_FILE_FORMAT_ERROR;
    }

    unsigned int id = 0;
    if (!parse_uint(tokens[0], &id)) {
        return STATUS_FILE_FORMAT_ERROR;
    }

    if (!validate_latin_string(tokens[1]) || !validate_latin_string(tokens[2])) {
        return STATUS_FILE_FORMAT_ERROR;
    }

    double salary = 0.0;
    if (!parse_nonnegative_double(tokens[3], &salary)) {
        return STATUS_FILE_FORMAT_ERROR;
    }

    char *name_copy = my_strdup(tokens[1]);
    if (name_copy == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    char *surname_copy = my_strdup(tokens[2]);
    if (surname_copy == NULL) {
        free(name_copy);
        return STATUS_MEMORY_ERROR;
    }

    emp->id = id;
    emp->name = name_copy;
    emp->surname = surname_copy;
    emp->salary = salary;
    return STATUS_OK;
}

status_code_t read_employees_from_file(const char *filepath, Employee **out_arr, size_t *out_count) {
    if (filepath == NULL || out_arr == NULL || out_count == NULL) {
        return STATUS_INVALID_ARGUMENT;
    }

    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        return STATUS_FILE_OPEN_ERROR;
    }

    size_t capacity = 16;
    size_t count = 0;
    Employee *arr = (Employee *)malloc(capacity * sizeof(Employee));
    if (arr == NULL) {
        fclose(f);
        return STATUS_MEMORY_ERROR;
    }

    char *line = NULL;
    status_code_t status = STATUS_OK;

    while ((status = read_dynamic_line(f, &line)) == STATUS_OK && line != NULL) {
        Employee temp_emp;
        temp_emp.name = NULL;
        temp_emp.surname = NULL;

        status = parse_employee_line(line, &temp_emp);
        free(line);

        if (status != STATUS_OK) {
            free_employees(arr, count);
            fclose(f);
            return status;
        }

        if (temp_emp.name == NULL) {
            continue;
        }

        if (count >= capacity) {
            size_t new_cap = capacity * 2;
            Employee *new_arr = (Employee *)realloc(arr, new_cap * sizeof(Employee));
            if (new_arr == NULL) {
                free(temp_emp.name);
                free(temp_emp.surname);
                free_employees(arr, count);
                fclose(f);
                return STATUS_MEMORY_ERROR;
            }
            arr = new_arr;
            capacity = new_cap;
        }

        arr[count++] = temp_emp;
    }

    fclose(f);

    if (status != STATUS_OK) {
        free_employees(arr, count);
        return status;
    }

    *out_arr = arr;
    *out_count = count;
    return STATUS_OK;
}

static int compare_double(double a, double b, double epsilon) {
    double diff = a - b;
    if ((diff < 0.0 ? -diff : diff) < epsilon) {
        return 0;
    }
    return (a > b) ? 1 : -1;
}

static int compare_employees_base(const Employee *e1, const Employee *e2) {
    double eps = 1e-9;
    int cmp_sal = compare_double(e1->salary, e2->salary, eps);
    if (cmp_sal != 0) {
        return cmp_sal;
    }

    int cmp_surname = my_strcmp(e1->surname, e2->surname);
    if (cmp_surname != 0) {
        return cmp_surname;
    }

    int cmp_name = my_strcmp(e1->name, e2->name);
    if (cmp_name != 0) {
        return cmp_name;
    }

    if (e1->id < e2->id) {
        return -1;
    }
    if (e1->id > e2->id) {
        return 1;
    }
    return 0;
}

static int compare_employees_asc(const void *a, const void *b) {
    return compare_employees_base((const Employee *)a, (const Employee *)b);
}

static int compare_employees_desc(const void *a, const void *b) {
    return compare_employees_base((const Employee *)b, (const Employee *)a);
}

status_code_t sort_employees(Employee *arr, size_t count, char flag) {
    if (arr == NULL && count > 0) {
        return STATUS_INVALID_ARGUMENT;
    }
    if (count <= 1) {
        return STATUS_OK;
    }

    if (flag == 'a') {
        qsort(arr, count, sizeof(Employee), compare_employees_asc);
    } else if (flag == 'd') {
        qsort(arr, count, sizeof(Employee), compare_employees_desc);
    } else {
        return STATUS_INVALID_ARGUMENT;
    }

    return STATUS_OK;
}

status_code_t write_employees_to_file(const char *filepath, const Employee *arr, size_t count) {
    if (filepath == NULL) {
        return STATUS_INVALID_ARGUMENT;
    }

    FILE *f = fopen(filepath, "w");
    if (f == NULL) {
        return STATUS_FILE_OPEN_ERROR;
    }

    for (size_t i = 0; i < count; ++i) {
        fprintf(f, "%u %s %s %.2f\n", arr[i].id, arr[i].name, arr[i].surname, arr[i].salary);
    }

    fclose(f);
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Ошибка: неверное количество аргументов.\n");
        printf("Использование: %s <входной_файл> <флаг (-a, /a, -d, /d)> <выходной_файл>\n", argv[0]);
        return 1;
    }

    const char *in_filepath = argv[1];
    const char *flag_str = argv[2];
    const char *out_filepath = argv[3];

    if ((flag_str[0] != '-' && flag_str[0] != '/') || (flag_str[1] != 'a' && flag_str[1] != 'd') || flag_str[2] != '\0') {
        printf("Ошибка: некорректный флаг '%s'. Ожидалось -a, /a, -d или /d.\n", flag_str);
        return 1;
    }

    char flag = flag_str[1];

    Employee *employees = NULL;
    size_t count = 0;

    status_code_t status = read_employees_from_file(in_filepath, &employees, &count);
    if (status == STATUS_FILE_OPEN_ERROR) {
        printf("Ошибка: невозможно открыть входной файл '%s'.\n", in_filepath);
        return 1;
    } else if (status == STATUS_FILE_FORMAT_ERROR) {
        printf("Ошибка: нарушен формат данных во входном файле.\n");
        return 1;
    } else if (status == STATUS_MEMORY_ERROR) {
        printf("Ошибка: не удалось выделить память.\n");
        return 1;
    } else if (status != STATUS_OK) {
        printf("Ошибка при чтении файла.\n");
        return 1;
    }

    status = sort_employees(employees, count, flag);
    if (status != STATUS_OK) {
        free_employees(employees, count);
        printf("Ошибка сортировки.\n");
        return 1;
    }

    status = write_employees_to_file(out_filepath, employees, count);
    free_employees(employees, count);

    if (status == STATUS_FILE_OPEN_ERROR) {
        printf("Ошибка: невозможно открыть выходной файл '%s' для записи.\n", out_filepath);
        return 1;
    } else if (status != STATUS_OK) {
        printf("Ошибка при записи выходного файла.\n");
        return 1;
    }

    printf("Успешно: обработано записей: %zu. Результат сохранен в '%s'.\n", count, out_filepath);
    return 0;
}