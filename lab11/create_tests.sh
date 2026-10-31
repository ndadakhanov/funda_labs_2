#!/bin/bash

mkdir -p tests

# Флаг -l (подсчет длины)
echo '-l Hello_World' > tests/args_task01_l_basic.txt
echo '/l ""' > tests/args_task01_l_empty.txt

# Флаг -r (реверс строки)
echo '-r abcdef_123' > tests/args_task01_r_basic.txt
echo '-r 1' > tests/args_task01_r_single.txt

# Флаг -u (нечетные позиции в верхний регистр)
echo '-u abcdefgh' > tests/args_task01_u_basic.txt
echo '-u 1a2b3c4d' > tests/args_task01_u_digits.txt

# Флаг -n (цифры -> буквы)
echo '-n a1B2!c3?4d' > tests/args_task01_n_basic.txt
echo '-n 987654321' > tests/args_task01_n_digits_only.txt

# Флаг -c (конкатенация: 2-я, 4-я, 5-я... в случайном порядке)
echo '-c first 42 second third fourth' > tests/args_task01_c_multiple.txt
echo '-c only_one 100' > tests/args_task01_c_single.txt
echo '-c test 0 another' > tests/args_task01_c_zero_seed.txt

# Тесты на ошибки и защиту от некорректного ввода
echo '-l' > tests/args_task01_err_no_string.txt
echo '-l first second_extra' > tests/args_task01_err_extra_arg.txt
echo '-z test' > tests/args_task01_err_unknown_flag.txt
echo '--long_flag test' > tests/args_task01_err_bad_flag_format.txt
echo '-c first' > tests/args_task01_err_c_missing_seed.txt
echo '-c first not_a_number' > tests/args_task01_err_c_str_seed.txt
echo '-c first -5 second' > tests/args_task01_err_c_negative_seed.txt
echo '-c first 42abc second' > tests/args_task01_err_c_trailing_chars.txt
echo '-c first 99999999999999999999999999999999999999 second' > tests/args_task01_err_c_overflow.txt

echo "Тестовые файлы успешно созданы в папке tests/"