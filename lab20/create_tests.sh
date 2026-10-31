#!/bin/bash

mkdir -p tests_args

# 1. Квадратный многочлен: f(x) = 1 + 2x + 3x^2, a = 1.0 -> g = [6, 8, 3]
echo "0.0001 1.0 2 1.0 2.0 3.0" > tests_args/args_task10_quadratic.txt

# 2. Кубический многочлен: f(x) = -2 + x - 3x^2 + x^3, a = 2.0 -> g = [-4, 1, 3, 1]
echo "0.0001 2.0 3 -2.0 1.0 -3.0 1.0" > tests_args/args_task10_cubic.txt

# 3. Многочлен нулевой степени (константа): f(x) = 42, a = 5.0 -> g = [42]
echo "0.0001 5.0 0 42.0" > tests_args/args_task10_degree0_const.txt

# 4. Точка a = 0 (тождественное переразложение, коэффициенты не должны измениться)
echo "0.0001 0.0 2 5.0 -3.0 2.0" > tests_args/args_task10_a_zero.txt

# 5. Отрицательный сдвиг: f(x) = 3 + 2x, a = -1.0 -> g(x+1) = 1 + 2(x+1)
echo "0.0001 -1.0 1 3.0 2.0" > tests_args/args_task10_negative_a.txt

# 6. Ошибки параметров (защита от некорректного ввода)
echo "-0.0001 1.0 2 1.0 2.0 3.0" > tests_args/args_task10_err_negative_eps.txt
echo "0.0 1.0 2 1.0 2.0 3.0" > tests_args/args_task10_err_zero_eps.txt
echo "not_an_eps 1.0 2 1.0 2.0 3.0" > tests_args/args_task10_err_eps_nan.txt
echo "0.0001 not_a 2 1.0 2.0 3.0" > tests_args/args_task10_err_a_nan.txt
echo "0.0001 1.0 -1 1.0" > tests_args/args_task10_err_negative_degree.txt
echo "0.0001 1.0 2 1.0 2.0" > tests_args/args_task10_err_missing_coeff.txt
echo "0.0001 1.0 2 1.0 2.0 3.0 4.0" > tests_args/args_task10_err_extra_coeff.txt
echo "0.0001 1.0 2 1.0 abc 3.0" > tests_args/args_task10_err_coeff_nan.txt
echo "0.0001 1.0 11 1 2 3 4 5 6 7 8 9 10 11 12" > tests_args/args_task10_err_degree_limit.txt

echo "Тестовые наборы аргументов для лабораторной 10 успешно созданы в папке tests_args/"