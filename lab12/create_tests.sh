#!/bin/bash

mkdir -p tests_data
mkdir -p tests_args
mkdir -p tests_out

# 1. Корректные входные данные
cat << 'EOF' > tests_data/valid_mixed.txt
10 Alice Smith 5000.00
5 Bob Smith 5000.00
8 Bob Smith 5000.00
1 Charlie Brown 2500.50
2 David Adams 8000.00
EOF

cat << 'EOF' > tests_data/valid_single.txt
42 Ivan Ivanov 12345.67
EOF

cat << 'EOF' > tests_data/valid_empty.txt
EOF

# 2. Некорректные входные данные (проверка защиты от дурака)
# Отрицательный ID
cat << 'EOF' > tests_data/err_negative_id.txt
-5 Alice Smith 5000.00
EOF

# Переполнение ID (число больше UINT_MAX)
cat << 'EOF' > tests_data/err_overflow_id.txt
999999999999999999999999999999999999 Alice Smith 5000.00
EOF

# Нелатинские буквы / цифры в имени
cat << 'EOF' > tests_data/err_name_digits.txt
1 Alice123 Smith 5000.00
EOF

# Отрицательная зарплата
cat << 'EOF' > tests_data/err_negative_salary.txt
1 Alice Smith -500.00
EOF

# Мусор вместо зарплаты
cat << 'EOF' > tests_data/err_salary_nan.txt
1 Alice Smith one_million
EOF

# Лишний столбец в строке
cat << 'EOF' > tests_data/err_extra_column.txt
1 Alice Smith 5000.00 EXTRA_DATA
EOF

# Неполная строка
cat << 'EOF' > tests_data/err_missing_column.txt
1 Alice Smith
EOF


# 3. Файлы аргументов для командной строки
echo "tests_data/valid_mixed.txt -a tests_out/out_mixed_asc.txt" > tests_args/args_task02_valid_asc.txt
echo "tests_data/valid_mixed.txt /d tests_out/out_mixed_desc.txt" > tests_args/args_task02_valid_desc.txt
echo "tests_data/valid_single.txt -a tests_out/out_single.txt" > tests_args/args_task02_valid_single.txt
echo "tests_data/valid_empty.txt -a tests_out/out_empty.txt" > tests_args/args_task02_valid_empty.txt

echo "tests_data/err_negative_id.txt -a tests_out/out_err.txt" > tests_args/args_task02_err_neg_id.txt
echo "tests_data/err_overflow_id.txt -a tests_out/out_err.txt" > tests_args/args_task02_err_ovf_id.txt
echo "tests_data/err_name_digits.txt -a tests_out/out_err.txt" > tests_args/args_task02_err_name.txt
echo "tests_data/err_negative_salary.txt -a tests_out/out_err.txt" > tests_args/args_task02_err_salary.txt
echo "tests_data/err_salary_nan.txt -a tests_out/out_err.txt" > tests_args/args_task02_err_salary_nan.txt
echo "tests_data/err_extra_column.txt -a tests_out/out_err.txt" > tests_args/args_task02_err_extra_col.txt
echo "tests_data/err_missing_column.txt -a tests_out/out_err.txt" > tests_args/args_task02_err_missing_col.txt

echo "tests_data/non_existent_file.txt -a tests_out/out_err.txt" > tests_args/args_task02_err_no_file.txt
echo "tests_data/valid_mixed.txt -x tests_out/out_err.txt" > tests_args/args_task02_err_bad_flag.txt
echo "tests_data/valid_mixed.txt" > tests_args/args_task02_err_few_args.txt

echo "Тестовые наборы успешно созданы!"