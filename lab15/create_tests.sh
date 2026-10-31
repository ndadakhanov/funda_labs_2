#!/bin/bash

mkdir -p tests_args

# 1. Римские числа (%Ro)
echo "-Ro 1994" > tests_args/args_task05_ro_1994.txt
echo "-Ro 4" > tests_args/args_task05_ro_4.txt
echo "-Ro -15" > tests_args/args_task05_ro_neg.txt

# 2. Цекендорфово представление (%Zr)
echo "-Zr 10" > tests_args/args_task05_zr_10.txt
echo "-Zr 1" > tests_args/args_task05_zr_1.txt
echo "-Zr 0" > tests_args/args_task05_zr_0.txt

# 3. Перевод в систему счисления (%Cv / %CV)
echo "-Cv 255 16" > tests_args/args_task05_cv_lower.txt
echo "-CV 255 16" > tests_args/args_task05_cv_upper.txt
echo "-Cv 10 2" > tests_args/args_task05_cv_bin.txt

# 4. Перевод из системы счисления в десятичную (%to / %TO)
echo "-to ff 16" > tests_args/args_task05_to_valid.txt
echo "-TO FF 16" > tests_args/args_task05_to_upper_valid.txt
echo "-to FF 16" > tests_args/args_task05_to_case_mismatch.txt
echo "-TO ff 16" > tests_args/args_task05_TO_case_mismatch.txt

# 5. Дамп памяти (%mi, %mu, %mf, %md)
echo "-mi 1" > tests_args/args_task05_mi.txt
echo "-mu 1" > tests_args/args_task05_mu.txt
echo "-mf 1.0" > tests_args/args_task05_mf.txt
echo "-md 1.0" > tests_args/args_task05_md.txt

# 6. Ошибки ввода (защита от дурака)
echo "-Ro not_a_number" > tests_args/args_task05_err_nan.txt
echo "-Ro 999999999999999999999999999999999999" > tests_args/args_task05_err_overflow.txt
echo "-Zr -5" > tests_args/args_task05_err_zr_neg.txt
echo "-Cv 10" > tests_args/args_task05_err_missing_base.txt
echo "-unknown_flag" > tests_args/args_task05_err_unknown.txt

echo "Тестовые наборы аргументов для лабораторной 5 успешно созданы!"