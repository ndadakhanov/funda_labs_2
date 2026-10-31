#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_MEMORY_ERROR,
    STATUS_FILE_OPEN_ERROR
} status_code_t;

typedef struct {
    size_t line;
    size_t col;
} MatchPos;

typedef struct {
    char *filepath;
    status_code_t status;
    MatchPos *matches;
    size_t match_count;
} FileSearchResult;

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

static char *unescape_string(const char *src) {
    if (src == NULL) {
        return NULL;
    }
    size_t len = my_strlen(src);
    char *res = (char *)malloc((len + 1) * sizeof(char));
    if (res == NULL) {
        return NULL;
    }
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        if (src[i] == '\\' && i + 1 < len) {
            if (src[i + 1] == 'n') {
                res[j++] = '\n';
                i++;
            } else if (src[i + 1] == 't') {
                res[j++] = '\t';
                i++;
            } else if (src[i + 1] == 'r') {
                res[j++] = '\r';
                i++;
            } else if (src[i + 1] == '\\') {
                res[j++] = '\\';
                i++;
            } else {
                res[j++] = src[i];
            }
        } else {
            res[j++] = src[i];
        }
    }
    res[j] = '\0';
    return res;
}

static status_code_t read_file_content(FILE *f, char **out_buf, size_t *out_len) {
    size_t capacity = 512;
    size_t length = 0;
    char *buf = (char *)malloc(capacity * sizeof(char));
    if (buf == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    size_t bytes_read = 0;
    while ((bytes_read = fread(buf + length, 1, capacity - length, f)) > 0) {
        length += bytes_read;
        if (length >= capacity) {
            size_t new_cap = capacity * 2;
            char *new_buf = (char *)realloc(buf, new_cap * sizeof(char));
            if (new_buf == NULL) {
                free(buf);
                return STATUS_MEMORY_ERROR;
            }
            buf = new_buf;
            capacity = new_cap;
        }
    }

    char *final_buf = (char *)realloc(buf, (length + 1) * sizeof(char));
    if (final_buf == NULL) {
        free(buf);
        return STATUS_MEMORY_ERROR;
    }
    final_buf[length] = '\0';
    *out_buf = final_buf;
    *out_len = length;
    return STATUS_OK;
}

static status_code_t search_in_buffer(const char *buf, size_t buf_len,
                                      const char *sub, size_t sub_len,
                                      MatchPos **out_matches, size_t *out_count) {
    if (sub_len == 0) {
        *out_matches = NULL;
        *out_count = 0;
        return STATUS_OK;
    }

    size_t cap = 8;
    size_t count = 0;
    MatchPos *matches = (MatchPos *)malloc(cap * sizeof(MatchPos));
    if (matches == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    size_t cur_line = 1;
    size_t cur_col = 1;

    for (size_t i = 0; i < buf_len; ++i) {
        if (i + sub_len <= buf_len) {
            int match = 1;
            for (size_t k = 0; k < sub_len; ++k) {
                if (buf[i + k] != sub[k]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                if (count >= cap) {
                    size_t new_cap = cap * 2;
                    MatchPos *new_m = (MatchPos *)realloc(matches, new_cap * sizeof(MatchPos));
                    if (new_m == NULL) {
                        free(matches);
                        return STATUS_MEMORY_ERROR;
                    }
                    matches = new_m;
                    cap = new_cap;
                }
                matches[count].line = cur_line;
                matches[count].col = cur_col;
                count++;
            }
        }

        if (buf[i] == '\n') {
            cur_line++;
            cur_col = 1;
        } else if (buf[i] != '\r') {
            cur_col++;
        }
    }

    *out_matches = matches;
    *out_count = count;
    return STATUS_OK;
}

void free_search_results(FileSearchResult *results, size_t file_count) {
    if (results == NULL) {
        return;
    }
    for (size_t i = 0; i < file_count; ++i) {
        free(results[i].filepath);
        free(results[i].matches);
    }
    free(results);
}

status_code_t search_in_files(FileSearchResult **out_results, const char *substr, size_t file_count, ...) {
    if (out_results == NULL || substr == NULL || file_count == 0) {
        return STATUS_INVALID_ARGUMENT;
    }

    size_t sub_len = my_strlen(substr);
    if (sub_len == 0) {
        return STATUS_INVALID_ARGUMENT;
    }

    FileSearchResult *results = (FileSearchResult *)malloc(file_count * sizeof(FileSearchResult));
    if (results == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    for (size_t i = 0; i < file_count; ++i) {
        results[i].filepath = NULL;
        results[i].status = STATUS_OK;
        results[i].matches = NULL;
        results[i].match_count = 0;
    }

    va_list args;
    va_start(args, file_count);

    for (size_t i = 0; i < file_count; ++i) {
        const char *fpath = va_arg(args, const char *);
        if (fpath == NULL) {
            va_end(args);
            free_search_results(results, file_count);
            return STATUS_INVALID_ARGUMENT;
        }

        results[i].filepath = my_strdup(fpath);
        if (results[i].filepath == NULL) {
            va_end(args);
            free_search_results(results, file_count);
            return STATUS_MEMORY_ERROR;
        }

        FILE *f = fopen(fpath, "rb");
        if (f == NULL) {
            results[i].status = STATUS_FILE_OPEN_ERROR;
            continue;
        }

        char *buf = NULL;
        size_t buf_len = 0;
        status_code_t r_status = read_file_content(f, &buf, &buf_len);
        fclose(f);

        if (r_status != STATUS_OK) {
            va_end(args);
            free_search_results(results, file_count);
            return r_status;
        }

        r_status = search_in_buffer(buf, buf_len, substr, sub_len,
                                    &results[i].matches, &results[i].match_count);
        free(buf);

        if (r_status != STATUS_OK) {
            va_end(args);
            free_search_results(results, file_count);
            return r_status;
        }
    }

    va_end(args);
    *out_results = results;
    return STATUS_OK;
}

void print_search_results(const FileSearchResult *results, size_t file_count) {
    if (results == NULL) {
        return;
    }
    for (size_t i = 0; i < file_count; ++i) {
        printf("Файл: %s\n", results[i].filepath);
        if (results[i].status == STATUS_FILE_OPEN_ERROR) {
            printf("  Статус: ошибка открытия файла (файл не существует или нет доступа).\n\n");
            continue;
        }
        if (results[i].match_count == 0) {
            printf("  Совпадений не найдено.\n\n");
        } else {
            printf("  Найдено совпадений: %zu\n", results[i].match_count);
            for (size_t m = 0; m < results[i].match_count; ++m) {
                printf("    - Строка %zu, символ %zu\n",
                       results[i].matches[m].line,
                       results[i].matches[m].col);
            }
            printf("\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Ошибка: недостаточно аргументов.\n");
        printf("Использование: %s <подстрока> <файл1> [файл2 ...]\n", argv[0]);
        return 1;
    }

    char *clean_substr = unescape_string(argv[1]);
    if (clean_substr == NULL) {
        printf("Ошибка: не удалось выделить память.\n");
        return 1;
    }
    if (my_strlen(clean_substr) == 0) {
        printf("Ошибка: искомая подстрока не должна быть пустой.\n");
        free(clean_substr);
        return 1;
    }

    size_t file_count = (size_t)(argc - 2);
    FileSearchResult *results = NULL;
    status_code_t status = STATUS_OK;

    switch (file_count) {
        case 1:
            status = search_in_files(&results, clean_substr, 1, argv[2]);
            break;
        case 2:
            status = search_in_files(&results, clean_substr, 2, argv[2], argv[3]);
            break;
        case 3:
            status = search_in_files(&results, clean_substr, 3, argv[2], argv[3], argv[4]);
            break;
        case 4:
            status = search_in_files(&results, clean_substr, 4, argv[2], argv[3], argv[4], argv[5]);
            break;
        case 5:
            status = search_in_files(&results, clean_substr, 5, argv[2], argv[3], argv[4], argv[5], argv[6]);
            break;
        case 6:
            status = search_in_files(&results, clean_substr, 6, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
            break;
        case 7:
            status = search_in_files(&results, clean_substr, 7, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8]);
            break;
        case 8:
            status = search_in_files(&results, clean_substr, 8, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9]);
            break;
        case 9:
            status = search_in_files(&results, clean_substr, 9, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10]);
            break;
        case 10:
            status = search_in_files(&results, clean_substr, 10, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], argv[11]);
            break;
        default:
            printf("Ошибка: поддерживается поиск максимум по 10 файлам за один запуск.\n");
            free(clean_substr);
            return 1;
    }

    if (status == STATUS_MEMORY_ERROR) {
        printf("Ошибка: не удалось выделить память.\n");
        free(clean_substr);
        return 1;
    } else if (status != STATUS_OK) {
        printf("Ошибка при выполнении поиска.\n");
        free(clean_substr);
        return 1;
    }

    print_search_results(results, file_count);
    free_search_results(results, file_count);
    free(clean_substr);

    return 0;
}