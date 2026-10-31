#!/bin/bash

mkdir -p tests_args

# 1. Корректные вычисления в разных системах счисления
echo "16 1A 02B C" > tests_args/args_task08_base16.txt
echo "2 1011 1101 1 0" > tests_args/args_task08_base2.txt
echo "36 ZZ 1" > tests_args/args_task08_base36.txt
echo "8 77 1" > tests_args/args_task08_base8.txt

# 2. Ведущие нули
echo "10 00000 00 0" > tests_args/args_task08_zeros_only.txt
echo "16 00001A 0000002B" > tests_args/args_task08_zeros_leading.txt

# 3. Длинная арифметика (числа из 50 знаков, не влезающие в 64-битные типы)
echo "10 99999999999999999999999999999999999999999999999999 1" > tests_args/args_task08_huge_number.txt

# 4. Ошибки и граничные случаи (защита от некорректного ввода)
echo "2 1012 1" > tests_args/args_task08_err_invalid_digit_base2.txt
echo "16 1G" > tests_args/args_task08_err_invalid_digit_base16.txt
echo "10 -5 10" > tests_args/args_task08_err_negative_num.txt
echo "1 10 20" > tests_args/args_task08_err_base_less_2.txt
echo "37 10 20" > tests_args/args_task08_err_base_greater_36.txt
echo "10" > tests_args/args_task08_err_missing_numbers.txt
echo "not_a_base 10 20" > tests_args/args_task08_err_base_not_number.txt

echo "Тестовые наборы для лабораторной 8 успешно созданы в папке tests_args/"