#!/bin/bash

mkdir -p tests_args
mkdir -p tests_data

# 1. Чтение римских чисел (%Ro)
echo "-Ro MCMXCIV" > tests_args/args_task06_ro_1994.txt
echo "-Ro IV" > tests_args/args_task06_ro_4.txt
echo "-Ro -XV" > tests_args/args_task06_ro_neg.txt

# 2. Чтение цекендорфова представления (%Zr)
echo "-Zr 010011" > tests_args/args_task06_zr_10.txt
echo "-Zr 10100011" > tests_args/args_task06_zr_25.txt
echo "-Zr 01" > tests_args/args_task06_zr_0.txt
echo "-Zr 11" > tests_args/args_task06_zr_1.txt

# 3. Чтение в произвольной системе счисления (%Cv / %CV)
echo "-Cv ff 16" > tests_args/args_task06_cv_lower_valid.txt
echo "-CV FF 16" > tests_args/args_task06_cv_upper_valid.txt
echo "-Cv 1010 2" > tests_args/args_task06_cv_bin.txt
echo "-Cv FF 16" > tests_args/args_task06_cv_case_mismatch.txt
echo "-CV ff 16" > tests_args/args_task06_CV_case_mismatch.txt

# 4. Чтение из файла через overfscanf
cat << 'EOF' > tests_data/file_input.txt
MCMXCIV 010011 ff
EOF
echo "-file tests_data/file_input.txt" > tests_args/args_task06_file.txt

# 5. Ошибки и защита от некорректного ввода
echo "-Ro NOT_A_ROMAN" > tests_args/args_task06_err_ro_invalid.txt
echo "-Zr 12345" > tests_args/args_task06_err_zr_invalid.txt
echo "-Cv ff" > tests_args/args_task06_err_missing_base.txt
echo "-file non_existent_file.txt" > tests_args/args_task06_err_no_file.txt
echo "-unknown_flag" > tests_args/args_task06_err_unknown.txt

echo "Тестовые данные для лабораторной 6 успешно созданы!"