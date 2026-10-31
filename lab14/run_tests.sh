#!/bin/bash

gcc -std=c99 -Wall -Wextra main.c -o a.out || { echo "Ошибка компиляции!"; exit 1; }

echo "================ ТЕСТИРОВАНИЕ ЛАБОРАТОРНОЙ 4 ================"

for file in tests_input/input_task04_*; do
    [ -e "$file" ] || continue
    echo "----------------------------------------"
    echo "Тест:    $file"
    echo "Команда: ./a.out < $file"
    echo "Вывод:"
    ./a.out < "$file"
    echo ""
done

echo "================ ВСЕ ТЕСТЫ ВЫПОЛНЕНЫ ================"