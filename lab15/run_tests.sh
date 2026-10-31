#!/bin/bash

gcc -std=c99 -Wall -Wextra main.c -o a.out || { echo "Ошибка компиляции!"; exit 1; }

echo "================ ТЕСТИРОВАНИЕ ЛАБОРАТОРНОЙ 5 (ПОФЛАГОВО) ================"

for file in tests_args/args_task05_*; do
    [ -e "$file" ] || continue
    args=$(tr -d '\r' < "$file")
    echo "----------------------------------------"
    echo "Тест:    $file"
    echo "Команда: ./a.out $args"
    echo "Вывод:"
    ./a.out $args
    echo ""
done

echo "================ КОМПЛЕКСНАЯ ДЕМОНСТРАЦИЯ overfprintf / oversprintf ================"
./a.out -demo
echo ""

if [ -f overfprintf_test.txt ]; then
    echo "--- Содержимое созданного файла overfprintf_test.txt ---"
    cat overfprintf_test.txt
    echo ""
fi

echo "================ ВСЕ ТЕСТЫ ВЫПОЛНЕНЫ ================"