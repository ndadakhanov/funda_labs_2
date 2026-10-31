#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_MEMORY_ERROR,
    STATUS_MATRIX_NOT_POSITIVE_DEFINITE,
    STATUS_FILE_ERROR,
    STATUS_INPUT_FORMAT_ERROR
} status_code_t;

typedef struct {
    double *coords;
    size_t dim;
} Vector;

typedef void (*universal_func_ptr_t)(void);

typedef struct {
    char norm_name[64];
    double max_norm;
    Vector *longest_vectors;
    size_t count;
} NormResult;

static int compare_double(double a, double b, double eps) {
    double diff = a - b;
    if (fabs(diff) < eps) {
        return 0;
    }
    return (a > b) ? 1 : -1;
}

double norm_inf(const Vector *v) {
    if (v == NULL || v->coords == NULL || v->dim == 0) {
        return 0.0;
    }
    double max_val = 0.0;
    for (size_t i = 0; i < v->dim; ++i) {
        double val = fabs(v->coords[i]);
        if (val > max_val) {
            max_val = val;
        }
    }
    return max_val;
}

double norm_p(const Vector *v, double p) {
    if (v == NULL || v->coords == NULL || v->dim == 0 || p < 1.0) {
        return 0.0;
    }
    double sum = 0.0;
    for (size_t i = 0; i < v->dim; ++i) {
        sum += pow(fabs(v->coords[i]), p);
    }
    return pow(sum, 1.0 / p);
}

double norm_matrix(const Vector *v, const double *matrix) {
    if (v == NULL || v->coords == NULL || matrix == NULL || v->dim == 0) {
        return 0.0;
    }
    size_t n = v->dim;
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double ax_i = 0.0;
        for (size_t j = 0; j < n; ++j) {
            ax_i += matrix[i * n + j] * v->coords[j];
        }
        sum += ax_i * v->coords[i];
    }
    if (sum < 0.0) {
        sum = 0.0;
    }
    return sqrt(sum);
}

static double compute_minor_det(const double *mat, size_t n, size_t k, double eps) {
    double *sub = (double *)malloc(k * k * sizeof(double));
    if (sub == NULL) {
        return 0.0;
    }

    for (size_t i = 0; i < k; ++i) {
        for (size_t j = 0; j < k; ++j) {
            sub[i * k + j] = mat[i * n + j];
        }
    }

    double det = 1.0;
    for (size_t col = 0; col < k; ++col) {
        size_t pivot = col;
        double max_val = fabs(sub[col * k + col]);
        for (size_t row = col + 1; row < k; ++row) {
            double v = fabs(sub[row * k + col]);
            if (v > max_val) {
                max_val = v;
                pivot = row;
            }
        }

        if (max_val < eps) {
            free(sub);
            return 0.0;
        }

        if (pivot != col) {
            for (size_t j = 0; j < k; ++j) {
                double tmp = sub[col * k + j];
                sub[col * k + j] = sub[pivot * k + j];
                sub[pivot * k + j] = tmp;
            }
            det = -det;
        }

        double diag = sub[col * k + col];
        det *= diag;

        for (size_t row = col + 1; row < k; ++row) {
            double factor = sub[row * k + col] / diag;
            for (size_t j = col + 1; j < k; ++j) {
                sub[row * k + j] -= factor * sub[col * k + j];
            }
        }
    }

    free(sub);
    return det;
}

int is_positive_definite(const double *mat, size_t n, double eps) {
    if (mat == NULL || n == 0) {
        return 0;
    }
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            if (fabs(mat[i * n + j] - mat[j * n + i]) > eps) {
                return 0;
            }
        }
    }
    for (size_t k = 1; k <= n; ++k) {
        double d = compute_minor_det(mat, n, k, eps);
        if (d <= eps) {
            return 0;
        }
    }
    return 1;
}

Vector create_vector(size_t dim) {
    Vector v;
    v.dim = dim;
    v.coords = (double *)malloc(dim * sizeof(double));
    return v;
}

void free_vector(Vector *v) {
    if (v != NULL && v->coords != NULL) {
        free(v->coords);
        v->coords = NULL;
    }
    if (v != NULL) {
        v->dim = 0;
    }
}

Vector clone_vector(const Vector *v) {
    Vector res;
    res.dim = (v != NULL) ? v->dim : 0;
    if (res.dim == 0 || v->coords == NULL) {
        res.coords = NULL;
        return res;
    }
    res.coords = (double *)malloc(res.dim * sizeof(double));
    if (res.coords != NULL) {
        for (size_t i = 0; i < res.dim; ++i) {
            res.coords[i] = v->coords[i];
        }
    }
    return res;
}

void free_norm_results(NormResult *results, size_t count) {
    if (results == NULL) {
        return;
    }
    for (size_t k = 0; k < count; ++k) {
        if (results[k].longest_vectors != NULL) {
            for (size_t i = 0; i < results[k].count; ++i) {
                free(results[k].longest_vectors[i].coords);
            }
            free(results[k].longest_vectors);
        }
    }
    free(results);
}

status_code_t find_longest_vectors(NormResult **out_results,
                                   double epsilon,
                                   size_t dim,
                                   size_t vector_count,
                                   size_t norm_count,
                                   ...) {
    if (out_results == NULL || dim == 0 || vector_count == 0 || norm_count == 0) {
        return STATUS_INVALID_ARGUMENT;
    }

    va_list args;
    va_start(args, norm_count);

    Vector *vectors = (Vector *)malloc(vector_count * sizeof(Vector));
    if (vectors == NULL) {
        va_end(args);
        return STATUS_MEMORY_ERROR;
    }

    for (size_t i = 0; i < vector_count; ++i) {
        vectors[i] = va_arg(args, Vector);
        if (vectors[i].dim != dim || vectors[i].coords == NULL) {
            free(vectors);
            va_end(args);
            return STATUS_INVALID_ARGUMENT;
        }
    }

    NormResult *results = (NormResult *)malloc(norm_count * sizeof(NormResult));
    if (results == NULL) {
        free(vectors);
        va_end(args);
        return STATUS_MEMORY_ERROR;
    }

    for (size_t k = 0; k < norm_count; ++k) {
        results[k].longest_vectors = NULL;
        results[k].count = 0;
        results[k].max_norm = 0.0;
        results[k].norm_name[0] = '\0';
    }

    for (size_t k = 0; k < norm_count; ++k) {
        universal_func_ptr_t f = va_arg(args, universal_func_ptr_t);
        if (f == NULL) {
            free_norm_results(results, k);
            free(vectors);
            va_end(args);
            return STATUS_INVALID_ARGUMENT;
        }

        double *norms = (double *)malloc(vector_count * sizeof(double));
        if (norms == NULL) {
            free_norm_results(results, k);
            free(vectors);
            va_end(args);
            return STATUS_MEMORY_ERROR;
        }

        if (f == (universal_func_ptr_t)norm_inf) {
            snprintf(results[k].norm_name, sizeof(results[k].norm_name), "Норма ||x||_inf (максимум по модулю)");
            for (size_t i = 0; i < vector_count; ++i) {
                norms[i] = ((double (*)(const Vector *))f)(&vectors[i]);
            }
        } else if (f == (universal_func_ptr_t)norm_p) {
            double p_val = va_arg(args, double);
            if (p_val < 1.0) {
                free(norms);
                free_norm_results(results, k);
                free(vectors);
                va_end(args);
                return STATUS_INVALID_ARGUMENT;
            }
            snprintf(results[k].norm_name, sizeof(results[k].norm_name), "Норма ||x||_p (p = %.2f)", p_val);
            for (size_t i = 0; i < vector_count; ++i) {
                norms[i] = ((double (*)(const Vector *, double))f)(&vectors[i], p_val);
            }
        } else if (f == (universal_func_ptr_t)norm_matrix) {
            const double *mat = va_arg(args, const double *);
            if (mat == NULL) {
                free(norms);
                free_norm_results(results, k);
                free(vectors);
                va_end(args);
                return STATUS_INVALID_ARGUMENT;
            }
            if (!is_positive_definite(mat, dim, epsilon)) {
                free(norms);
                free_norm_results(results, k);
                free(vectors);
                va_end(args);
                return STATUS_MATRIX_NOT_POSITIVE_DEFINITE;
            }
            snprintf(results[k].norm_name, sizeof(results[k].norm_name), "Матричная норма ||x||_A = sqrt((Ax, x))");
            for (size_t i = 0; i < vector_count; ++i) {
                norms[i] = ((double (*)(const Vector *, const double *))f)(&vectors[i], mat);
            }
        } else {
            snprintf(results[k].norm_name, sizeof(results[k].norm_name), "Пользовательская норма");
            for (size_t i = 0; i < vector_count; ++i) {
                norms[i] = ((double (*)(const Vector *))f)(&vectors[i]);
            }
        }

        double max_norm = norms[0];
        for (size_t i = 1; i < vector_count; ++i) {
            if (compare_double(norms[i], max_norm, epsilon) > 0) {
                max_norm = norms[i];
            }
        }

        size_t match_count = 0;
        for (size_t i = 0; i < vector_count; ++i) {
            if (compare_double(norms[i], max_norm, epsilon) == 0) {
                match_count++;
            }
        }

        results[k].longest_vectors = (Vector *)malloc(match_count * sizeof(Vector));
        if (results[k].longest_vectors == NULL) {
            free(norms);
            free_norm_results(results, k);
            free(vectors);
            va_end(args);
            return STATUS_MEMORY_ERROR;
        }

        size_t idx = 0;
        for (size_t i = 0; i < vector_count; ++i) {
            if (compare_double(norms[i], max_norm, epsilon) == 0) {
                results[k].longest_vectors[idx] = clone_vector(&vectors[i]);
                if (results[k].longest_vectors[idx].coords == NULL) {
                    for (size_t j = 0; j < idx; ++j) {
                        free(results[k].longest_vectors[j].coords);
                    }
                    free(results[k].longest_vectors);
                    free(norms);
                    free_norm_results(results, k);
                    free(vectors);
                    va_end(args);
                    return STATUS_MEMORY_ERROR;
                }
                idx++;
            }
        }

        results[k].count = match_count;
        results[k].max_norm = max_norm;
        free(norms);
    }

    va_end(args);
    free(vectors);

    *out_results = results;
    return STATUS_OK;
}

void print_norm_results(const NormResult *results, size_t norm_count) {
    if (results == NULL) {
        return;
    }
    for (size_t k = 0; k < norm_count; ++k) {
        printf("========================================\n");
        printf("▶ %s\n", results[k].norm_name);
        printf("  Максимальная длина (норма): %.6f\n", results[k].max_norm);
        printf("  Количество самых длинных векторов: %zu\n", results[k].count);
        for (size_t i = 0; i < results[k].count; ++i) {
            printf("    Вектор %zu: (", i + 1);
            for (size_t d = 0; d < results[k].longest_vectors[i].dim; ++d) {
                printf("%.4f%s", results[k].longest_vectors[i].coords[d],
                       (d + 1 < results[k].longest_vectors[i].dim) ? ", " : "");
            }
            printf(")\n");
        }
        printf("\n");
    }
}

void run_demonstration(void) {
    printf("=== ДЕМОНСТРАЦИЯ ЗАДАНИЯ 7 (ПОИСК САМЫХ ДЛИННЫХ ВЕКТОРОВ) ===\n\n");
    size_t dim = 3;
    size_t vector_count = 4;

    double c1[3] = {1.0, 2.0, 3.0};
    double c2[3] = {3.0, 2.0, 1.0};
    double c3[3] = {0.0, 1.0, 0.0};
    double c4[3] = {-3.0, -2.0, 1.0};

    Vector v1 = {c1, dim};
    Vector v2 = {c2, dim};
    Vector v3 = {c3, dim};
    Vector v4 = {c4, dim};

    printf("Переданные векторы (размерность n = %zu):\n", dim);
    printf("  v1 = (1.0, 2.0, 3.0)\n");
    printf("  v2 = (3.0, 2.0, 1.0)\n");
    printf("  v3 = (0.0, 1.0, 0.0)\n");
    printf("  v4 = (-3.0, -2.0, 1.0)\n\n");

    double A[9] = {
        2.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    };
    printf("Положительно определенная матрица A (3x3):\n");
    printf("  [ 2.0  0.0  0.0 ]\n");
    printf("  [ 0.0  1.0  0.0 ]\n");
    printf("  [ 0.0  0.0  1.0 ]\n\n");

    NormResult *results = NULL;
    size_t norm_count = 3;
    status_code_t st = find_longest_vectors(&results,
                                            1e-9,
                                            dim,
                                            vector_count,
                                            norm_count,
                                            v1, v2, v3, v4,
                                            (universal_func_ptr_t)norm_inf,
                                            (universal_func_ptr_t)norm_p, 2.0,
                                            (universal_func_ptr_t)norm_matrix, A);

    if (st == STATUS_OK) {
        print_norm_results(results, norm_count);
        free_norm_results(results, norm_count);
    } else {
        printf("Ошибка при вычислении норм (код: %d).\n", st);
    }
}

status_code_t run_from_file(FILE *f) {
    if (f == NULL) {
        return STATUS_FILE_ERROR;
    }

    size_t dim = 0;
    double p_val = 0.0;
    if (fscanf(f, "%zu %lf", &dim, &p_val) != 2 || dim == 0 || p_val < 1.0) {
        return STATUS_INPUT_FORMAT_ERROR;
    }

    double *matrix = (double *)malloc(dim * dim * sizeof(double));
    if (matrix == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    for (size_t i = 0; i < dim * dim; ++i) {
        if (fscanf(f, "%lf", &matrix[i]) != 1) {
            free(matrix);
            return STATUS_INPUT_FORMAT_ERROR;
        }
    }

    size_t vector_count = 0;
    if (fscanf(f, "%zu", &vector_count) != 1 || vector_count == 0) {
        free(matrix);
        return STATUS_INPUT_FORMAT_ERROR;
    }

    Vector *vecs = (Vector *)malloc(vector_count * sizeof(Vector));
    if (vecs == NULL) {
        free(matrix);
        return STATUS_MEMORY_ERROR;
    }

    for (size_t i = 0; i < vector_count; ++i) {
        vecs[i] = create_vector(dim);
        if (vecs[i].coords == NULL) {
            for (size_t j = 0; j < i; ++j) {
                free_vector(&vecs[j]);
            }
            free(vecs);
            free(matrix);
            return STATUS_MEMORY_ERROR;
        }
        for (size_t d = 0; d < dim; ++d) {
            if (fscanf(f, "%lf", &vecs[i].coords[d]) != 1) {
                for (size_t j = 0; j <= i; ++j) {
                    free_vector(&vecs[j]);
                }
                free(vecs);
                free(matrix);
                return STATUS_INPUT_FORMAT_ERROR;
            }
        }
    }

    NormResult *results = NULL;
    size_t norm_count = 3;
    status_code_t st = STATUS_OK;

    switch (vector_count) {
        case 1:
            st = find_longest_vectors(&results, 1e-9, dim, 1, norm_count,
                                      vecs[0],
                                      (universal_func_ptr_t)norm_inf,
                                      (universal_func_ptr_t)norm_p, p_val,
                                      (universal_func_ptr_t)norm_matrix, matrix);
            break;
        case 2:
            st = find_longest_vectors(&results, 1e-9, dim, 2, norm_count,
                                      vecs[0], vecs[1],
                                      (universal_func_ptr_t)norm_inf,
                                      (universal_func_ptr_t)norm_p, p_val,
                                      (universal_func_ptr_t)norm_matrix, matrix);
            break;
        case 3:
            st = find_longest_vectors(&results, 1e-9, dim, 3, norm_count,
                                      vecs[0], vecs[1], vecs[2],
                                      (universal_func_ptr_t)norm_inf,
                                      (universal_func_ptr_t)norm_p, p_val,
                                      (universal_func_ptr_t)norm_matrix, matrix);
            break;
        case 4:
            st = find_longest_vectors(&results, 1e-9, dim, 4, norm_count,
                                      vecs[0], vecs[1], vecs[2], vecs[3],
                                      (universal_func_ptr_t)norm_inf,
                                      (universal_func_ptr_t)norm_p, p_val,
                                      (universal_func_ptr_t)norm_matrix, matrix);
            break;
        case 5:
            st = find_longest_vectors(&results, 1e-9, dim, 5, norm_count,
                                      vecs[0], vecs[1], vecs[2], vecs[3], vecs[4],
                                      (universal_func_ptr_t)norm_inf,
                                      (universal_func_ptr_t)norm_p, p_val,
                                      (universal_func_ptr_t)norm_matrix, matrix);
            break;
        default:
            st = find_longest_vectors(&results, 1e-9, dim, 4, norm_count,
                                      vecs[0], vecs[1], vecs[2], vecs[3],
                                      (universal_func_ptr_t)norm_inf,
                                      (universal_func_ptr_t)norm_p, p_val,
                                      (universal_func_ptr_t)norm_matrix, matrix);
            break;
    }

    if (st == STATUS_OK) {
        print_norm_results(results, norm_count);
        free_norm_results(results, norm_count);
    }

    for (size_t i = 0; i < vector_count; ++i) {
        free_vector(&vecs[i]);
    }
    free(vecs);
    free(matrix);

    return st;
}

int main(int argc, char *argv[]) {
    if (argc == 1 || (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'd')) {
        run_demonstration();
        return 0;
    }

    const char *filepath = argv[1];
    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        printf("Ошибка: невозможно открыть файл '%s'.\n", filepath);
        return 1;
    }

    status_code_t st = run_from_file(f);
    fclose(f);

    if (st == STATUS_MATRIX_NOT_POSITIVE_DEFINITE) {
        printf("Ошибка: матрица A не является положительно определенной.\n");
        return 1;
    } else if (st == STATUS_INPUT_FORMAT_ERROR) {
        printf("Ошибка: нарушен формат входных данных.\n");
        return 1;
    } else if (st == STATUS_INVALID_ARGUMENT) {
        printf("Ошибка: некорректные аргументы (размерность или параметр p < 1).\n");
        return 1;
    } else if (st == STATUS_MEMORY_ERROR) {
        printf("Ошибка: не удалось выделить память.\n");
        return 1;
    } else if (st != STATUS_OK) {
        printf("Ошибка при обработке векторов.\n");
        return 1;
    }

    return 0;
}