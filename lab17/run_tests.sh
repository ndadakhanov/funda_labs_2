#!/bin/bash

gcc -std=c99 -Wall -Wextra main.c -lm -o a.out || { echo "Ошибка компиляции!"; exit 1; }

echo "================ ТЕСТИРОВАНИЕ ЛАБОРАТОРНОЙ 7 ================"

for file in tests_args/args_task07_*; do
    [ -e "$file" ] || continue
    args=$(tr -d '\r' < "$file")
    echo "----------------------------------------"
    echo "Тест:    $file"
    echo "Команда: ./a.out $args"
    echo "Вывод:"
    ./a.out $args
    echo ""
done

echo "================ КОМПЛЕКСНАЯ ВСТРОЕННАЯ ДЕМОНСТРАЦИЯ ================"
./a.out -demo
echo ""

echo "================ ВСЕ ТЕСТЫ ВЫПОЛНЕНЫ ================"