#!/bin/bash

gcc -std=c99 -Wall -Wextra main.c -o a.out || { echo "Ошибка компиляции!"; exit 1; }

echo "================ ТЕСТИРОВАНИЕ ЛАБОРАТОРНОЙ 2 ================"

for file in tests_args/args_task02_*; do
    [ -e "$file" ] || continue
    args=$(tr -d '\r' < "$file")
    echo "----------------------------------------"
    echo "Тест:    $file"
    echo "Команда: ./a.out $args"
    echo "Вывод:"
    ./a.out $args
    echo ""
done

echo "================ СОДЕРЖИМОЕ ВЫХОДНЫХ ФАЙЛОВ ПОСЛЕ СОРТИРОВКИ ================"
if [ -f tests_out/out_mixed_asc.txt ]; then
    echo "--- out_mixed_asc.txt (По возрастанию) ---"
    cat tests_out/out_mixed_asc.txt
    echo ""
fi

if [ -f tests_out/out_mixed_desc.txt ]; then
    echo "--- out_mixed_desc.txt (По убыванию) ---"
    cat tests_out/out_mixed_desc.txt
    echo ""
fi

echo "================ ВСЕ ТЕСТЫ ВЫПОЛНЕНЫ ================"