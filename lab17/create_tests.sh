#!/bin/bash

mkdir -p tests_data
mkdir -p tests_args

# 1. Корректный тест (3D векторы, p = 2.0, единичная матрица с масштабом 2 по x)
cat << 'EOF' > tests_data/data_task07_valid_3d.txt
3 2.0
2.0 0.0 0.0
0.0 1.0 0.0
0.0 0.0 1.0
4
1.0 2.0 3.0
3.0 2.0 1.0
0.0 1.0 0.0
-3.0 -2.0 1.0
EOF
echo "tests_data/data_task07_valid_3d.txt" > tests_args/args_task07_valid_3d.txt

# 2. Корректный тест в 2D (p = 3.0)
cat << 'EOF' > tests_data/data_task07_valid_2d.txt
2 3.0
3.0 0.0
0.0 3.0
3
1.0 5.0
5.0 1.0
-5.0 -1.0
EOF
echo "tests_data/data_task07_valid_2d.txt" > tests_args/args_task07_valid_2d.txt

# 3. Ошибка: матрица не является положительно определенной (отрицательный диагональный элемент)
cat << 'EOF' > tests_data/data_task07_err_not_posdef.txt
2 2.0
-1.0 0.0
0.0 2.0
2
1.0 2.0
3.0 4.0
EOF
echo "tests_data/data_task07_err_not_posdef.txt" > tests_args/args_task07_err_not_posdef.txt

# 4. Ошибка: матрица не симметрична
cat << 'EOF' > tests_data/data_task07_err_not_symmetric.txt
2 2.0
1.0 2.0
3.0 4.0
2
1.0 2.0
3.0 4.0
EOF
echo "tests_data/data_task07_err_not_symmetric.txt" > tests_args/args_task07_err_not_symmetric.txt

# 5. Ошибка: параметр p < 1
cat << 'EOF' > tests_data/data_task07_err_bad_p.txt
2 0.5
2.0 0.0
0.0 2.0
2
1.0 2.0
3.0 4.0
EOF
echo "tests_data/data_task07_err_bad_p.txt" > tests_args/args_task07_err_bad_p.txt

# 6. Ошибка: поврежденный формат файла
cat << 'EOF' > tests_data/data_task07_err_corrupted.txt
3 2.0
NOT_A_NUMBER 0.0 0.0
EOF
echo "tests_data/data_task07_err_corrupted.txt" > tests_args/args_task07_err_corrupted.txt

# 7. Ошибка: несуществующий файл
echo "tests_data/non_existent_file.txt" > tests_args/args_task07_err_no_file.txt

echo "Тестовые наборы для лабораторной 7 успешно созданы!"