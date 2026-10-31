#!/bin/bash

mkdir -p tests_args

# 1. Корректные тесты для различных оснований
echo "2 0.000000001 0.5 0.25 0.2 0.375" > tests_args/args_task09_base2.txt
echo "10 0.000000001 0.2 0.05 0.0016" > tests_args/args_task09_base10.txt
echo "3 0.000000001 0.5 0.3333333333333333" > tests_args/args_task09_base3.txt
echo "6 0.000000001 0.5 0.3333333333333333 0.2" > tests_args/args_task09_base6.txt

# 2. Ошибки диапазона основания системы счисления
echo "1 0.000000001 0.5" > tests_args/args_task09_err_base_less_2.txt
echo "37 0.000000001 0.5" > tests_args/args_task09_err_base_greater_36.txt
echo "not_a_base 0.000000001 0.5" > tests_args/args_task09_err_base_nan.txt

# 3. Ошибки точности epsilon (eps <= 0 или не число)
echo "2 -0.0001 0.5" > tests_args/args_task09_err_negative_eps.txt
echo "2 0.0 0.5" > tests_args/args_task09_err_zero_eps.txt
echo "2 not_an_eps 0.5" > tests_args/args_task09_err_eps_nan.txt

# 4. Ошибки диапазона дробей (дробь должна быть строго в (0; 1))
echo "2 0.000000001 0.0" > tests_args/args_task09_err_frac_zero.txt
echo "2 0.000000001 1.0" > tests_args/args_task09_err_frac_one.txt
echo "2 0.000000001 -0.5" > tests_args/args_task09_err_frac_neg.txt
echo "2 0.000000001 1.5" > tests_args/args_task09_err_frac_greater_one.txt
echo "2 0.000000001 abc" > tests_args/args_task09_err_frac_nan.txt

# 5. Недостаточное количество аргументов
echo "2 0.000000001" > tests_args/args_task09_err_no_fractions.txt

echo "Тестовые наборы для лабораторной 9 успешно созданы в папке tests_args/"