#!/bin/bash

gcc -std=c99 -Wall -Wextra main.c -lm -o a.out || { echo "Ошибка компиляции!"; exit 1; }

echo "================ ТЕСТИРОВАНИЕ ЛАБОРАТОРНОЙ 10 ================"

for file in tests_args/args_task10_*; do
    [ -e "$file" ] || continue
    args=$(tr -d '\r' < "$file")
    echo "----------------------------------------"
    echo "Тест:    $file"
    echo "Команда: ./a.out $args"
    echo "Вывод:"
    ./a.out $args
    echo ""
done

echo "================ ВСТРОЕННАЯ КОМПЛЕКСНАЯ ДЕМОНСТРАЦИЯ ================"
./a.out -demo
echo ""

echo "================ ВСЕ ТЕСТЫ ВЫПОЛНЕНЫ ================"