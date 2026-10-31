#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_MEMORY_ERROR,
    STATUS_INPUT_FORMAT_ERROR
} status_code_t;

typedef struct {
    char *filename;
    size_t size;
    size_t importance;
} FileInfo;

typedef struct {
    FileInfo *files;
    size_t count;
    size_t total_size;
    size_t total_importance;
} BackupResult;

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

static int is_space(char c) {
    return (c == ' ' || c == '\t');
}

static int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

static int parse_size_t(const char *str, size_t *out_val) {
    if (str == NULL || out_val == NULL || *str == '\0') {
        return 0;
    }
    if (*str == '+') {
        str++;
    }
    if (*str == '\0') {
        return 0;
    }
    size_t res = 0;
    while (*str != '\0') {
        if (!is_digit(*str)) {
            return 0;
        }
        size_t digit = (size_t)(*str - '0');
        if (res > (SIZE_MAX - digit) / 10) {
            return 0;
        }
        res = res * 10 + digit;
        str++;
    }
    *out_val = res;
    return 1;
}

static status_code_t read_dynamic_line(FILE *f, char **out_line) {
    size_t capacity = 64;
    size_t length = 0;
    char *buf = (char *)malloc(capacity * sizeof(char));
    if (buf == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    int c;
    while ((c = fgetc(f)) != EOF && c != '\n') {
        if (c == '\r') {
            continue;
        }
        if (length + 1 >= capacity) {
            size_t new_cap = capacity * 2;
            char *new_buf = (char *)realloc(buf, new_cap * sizeof(char));
            if (new_buf == NULL) {
                free(buf);
                return STATUS_MEMORY_ERROR;
            }
            buf = new_buf;
            capacity = new_cap;
        }
        buf[length++] = (char)c;
    }

    if (c == EOF && length == 0) {
        free(buf);
        *out_line = NULL;
        return STATUS_OK;
    }

    buf[length] = '\0';
    *out_line = buf;
    return STATUS_OK;
}

static int tokenize_line(char *line, char **tokens, int max_tokens) {
    int count = 0;
    char *p = line;
    while (*p != '\0') {
        while (*p != '\0' && is_space(*p)) {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        if (count < max_tokens) {
            tokens[count++] = p;
        } else {
            return count + 1;
        }
        while (*p != '\0' && !is_space(*p)) {
            p++;
        }
        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }
    return count;
}

void free_file_info_array(FileInfo *arr, size_t count) {
    if (arr == NULL) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        free(arr[i].filename);
    }
    free(arr);
}

void free_backup_result(BackupResult *result) {
    if (result == NULL) {
        return;
    }
    if (result->files != NULL) {
        for (size_t i = 0; i < result->count; ++i) {
            free(result->files[i].filename);
        }
        free(result->files);
        result->files = NULL;
    }
    result->count = 0;
    result->total_size = 0;
    result->total_importance = 0;
}

status_code_t read_backup_input(FILE *in, FileInfo **out_files, size_t *out_n, size_t *out_capacity) {
    if (in == NULL || out_files == NULL || out_n == NULL || out_capacity == NULL) {
        return STATUS_INVALID_ARGUMENT;
    }

    char *line = NULL;
    status_code_t st = STATUS_OK;

    while ((st = read_dynamic_line(in, &line)) == STATUS_OK && line != NULL) {
        char *tokens[3];
        int tc = tokenize_line(line, tokens, 2);
        if (tc == 0) {
            free(line);
            continue;
        }
        if (tc != 2) {
            free(line);
            return STATUS_INPUT_FORMAT_ERROR;
        }

        size_t n = 0;
        size_t cap = 0;
        if (!parse_size_t(tokens[0], &n) || !parse_size_t(tokens[1], &cap) || n == 0) {
            free(line);
            return STATUS_INPUT_FORMAT_ERROR;
        }

        *out_n = n;
        *out_capacity = cap;
        free(line);
        break;
    }

    if (st != STATUS_OK) {
        return st;
    }
    if (line == NULL && *out_n == 0) {
        return STATUS_INPUT_FORMAT_ERROR;
    }

    size_t target_count = *out_n;
    FileInfo *files = (FileInfo *)malloc(target_count * sizeof(FileInfo));
    if (files == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    size_t read_count = 0;
    while (read_count < target_count && (st = read_dynamic_line(in, &line)) == STATUS_OK && line != NULL) {
        char *tokens[4];
        int tc = tokenize_line(line, tokens, 3);
        if (tc == 0) {
            free(line);
            continue;
        }
        if (tc != 3) {
            free(line);
            free_file_info_array(files, read_count);
            return STATUS_INPUT_FORMAT_ERROR;
        }

        size_t f_size = 0;
        size_t f_imp = 0;
        if (!parse_size_t(tokens[1], &f_size) || !parse_size_t(tokens[2], &f_imp) || f_size == 0) {
            free(line);
            free_file_info_array(files, read_count);
            return STATUS_INPUT_FORMAT_ERROR;
        }

        for (size_t j = 0; j < read_count; ++j) {
            if (my_strcmp(files[j].filename, tokens[0]) == 0) {
                free(line);
                free_file_info_array(files, read_count);
                return STATUS_INPUT_FORMAT_ERROR;
            }
        }

        char *name_copy = my_strdup(tokens[0]);
        if (name_copy == NULL) {
            free(line);
            free_file_info_array(files, read_count);
            return STATUS_MEMORY_ERROR;
        }

        files[read_count].filename = name_copy;
        files[read_count].size = f_size;
        files[read_count].importance = f_imp;
        read_count++;

        free(line);
    }

    if (st != STATUS_OK) {
        free_file_info_array(files, read_count);
        return st;
    }

    if (read_count != target_count) {
        free_file_info_array(files, read_count);
        return STATUS_INPUT_FORMAT_ERROR;
    }

    while ((st = read_dynamic_line(in, &line)) == STATUS_OK && line != NULL) {
        char *dummy[1];
        int tc = tokenize_line(line, dummy, 0);
        if (tc > 0) {
            free(line);
            free_file_info_array(files, target_count);
            return STATUS_INPUT_FORMAT_ERROR;
        }
        free(line);
    }

    *out_files = files;
    return STATUS_OK;
}

status_code_t select_backup_files(const FileInfo *files, size_t file_count, size_t capacity, BackupResult *out_result) {
    if (out_result == NULL || (files == NULL && file_count > 0)) {
        return STATUS_INVALID_ARGUMENT;
    }

    out_result->files = NULL;
    out_result->count = 0;
    out_result->total_size = 0;
    out_result->total_importance = 0;

    if (file_count == 0 || capacity == 0) {
        return STATUS_OK;
    }

    size_t rows = file_count + 1;
    size_t cols = capacity + 1;

    if (cols != 0 && rows > SIZE_MAX / cols) {
        return STATUS_MEMORY_ERROR;
    }
    size_t total_elements = rows * cols;
    if (total_elements > SIZE_MAX / sizeof(size_t)) {
        return STATUS_MEMORY_ERROR;
    }

    size_t *dp = (size_t *)malloc(total_elements * sizeof(size_t));
    if (dp == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    for (size_t w = 0; w <= capacity; ++w) {
        dp[w] = 0;
    }

    for (size_t i = 1; i <= file_count; ++i) {
        size_t cur_sz = files[i - 1].size;
        size_t cur_imp = files[i - 1].importance;
        size_t prev_row = (i - 1) * cols;
        size_t cur_row = i * cols;

        for (size_t w = 0; w <= capacity; ++w) {
            size_t without_item = dp[prev_row + w];
            if (w >= cur_sz) {
                size_t with_item = dp[prev_row + (w - cur_sz)] + cur_imp;
                dp[cur_row + w] = (with_item > without_item) ? with_item : without_item;
            } else {
                dp[cur_row + w] = without_item;
            }
        }
    }

    int *selected = (int *)calloc(file_count, sizeof(int));
    if (selected == NULL) {
        free(dp);
        return STATUS_MEMORY_ERROR;
    }

    size_t w = capacity;
    size_t selected_count = 0;
    for (size_t i = file_count; i > 0; --i) {
        size_t cur_sz = files[i - 1].size;
        size_t cur_imp = files[i - 1].importance;
        size_t prev_row = (i - 1) * cols;
        size_t cur_row = i * cols;

        if (w >= cur_sz && dp[cur_row + w] == dp[prev_row + (w - cur_sz)] + cur_imp) {
            if (dp[cur_row + w] != dp[prev_row + w]) {
                selected[i - 1] = 1;
                selected_count++;
                w -= cur_sz;
            }
        }
    }

    free(dp);

    if (selected_count == 0) {
        free(selected);
        return STATUS_OK;
    }

    FileInfo *res_files = (FileInfo *)malloc(selected_count * sizeof(FileInfo));
    if (res_files == NULL) {
        free(selected);
        return STATUS_MEMORY_ERROR;
    }

    size_t idx = 0;
    size_t total_sz = 0;
    size_t total_imp = 0;
    for (size_t i = 0; i < file_count; ++i) {
        if (selected[i]) {
            res_files[idx].filename = my_strdup(files[i].filename);
            if (res_files[idx].filename == NULL) {
                for (size_t j = 0; j < idx; ++j) {
                    free(res_files[j].filename);
                }
                free(res_files);
                free(selected);
                return STATUS_MEMORY_ERROR;
            }
            res_files[idx].size = files[i].size;
            res_files[idx].importance = files[i].importance;
            total_sz += files[i].size;
            total_imp += files[i].importance;
            idx++;
        }
    }

    free(selected);

    out_result->files = res_files;
    out_result->count = selected_count;
    out_result->total_size = total_sz;
    out_result->total_importance = total_imp;
    return STATUS_OK;
}

void print_backup_result(const BackupResult *result) {
    if (result == NULL) {
        return;
    }
    printf("TOTAL_SIZE %zu\n", result->total_size);
    printf("TOTAL_IMPORTANCE %zu\n", result->total_importance);
    printf("FILES %zu\n", result->count);
    for (size_t i = 0; i < result->count; ++i) {
        printf("%s\n", result->files[i].filename);
    }
}

int main(void) {
    FileInfo *files = NULL;
    size_t file_count = 0;
    size_t capacity = 0;

    status_code_t status = read_backup_input(stdin, &files, &file_count, &capacity);
    if (status == STATUS_INPUT_FORMAT_ERROR) {
        printf("Ошибка: некорректный формат входных данных.\n");
        return 1;
    } else if (status == STATUS_MEMORY_ERROR) {
        printf("Ошибка: не удалось выделить память.\n");
        return 1;
    } else if (status != STATUS_OK) {
        printf("Ошибка при чтении данных.\n");
        return 1;
    }

    BackupResult result;
    status = select_backup_files(files, file_count, capacity, &result);
    free_file_info_array(files, file_count);

    if (status == STATUS_MEMORY_ERROR) {
        printf("Ошибка: не удалось выделить память для решения задачи.\n");
        return 1;
    } else if (status != STATUS_OK) {
        printf("Ошибка при выборе файлов для резервного копирования.\n");
        return 1;
    }

    print_backup_result(&result);
    free_backup_result(&result);

    return 0;
}