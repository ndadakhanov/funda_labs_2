#!/bin/bash

mkdir -p tests_input

# 1. Корректный базовый тест (выбираются оптимальные файлы)
cat << 'EOF' > tests_input/input_task04_basic.txt
4 50
fileA.txt 10 60
fileB.txt 20 100
fileC.txt 30 120
fileD.txt 40 80
EOF

# 2. Вместимость накопителя равна 0 (должен получиться пустой набор)
cat << 'EOF' > tests_input/input_task04_zero_capacity.txt
2 0
doc.pdf 10 50
img.png 20 80
EOF

# 3. Ни один файл не влезает в накопитель (пустой набор)
cat << 'EOF' > tests_input/input_task04_none_fit.txt
2 10
big_archive.zip 50 100
heavy_video.mp4 20 300
EOF

# 4. Все файлы влезают целиком
cat << 'EOF' > tests_input/input_task04_all_fit.txt
3 100
notes.txt 10 10
music.mp3 20 30
photo.raw 30 50
EOF

# 5. Тесты на дурака (ошибки валидации)
# Пустой ввод
cat << 'EOF' > tests_input/input_task04_err_empty.txt
EOF

# Число файлов n = 0 (по условию n > 0)
cat << 'EOF' > tests_input/input_task04_err_zero_n.txt
0 100
EOF

# Отрицательная вместимость накопителя
cat << 'EOF' > tests_input/input_task04_err_neg_cap.txt
2 -50
file1.txt 10 10
file2.txt 20 20
EOF

# Размер файла равен 0 (по условию size > 0)
cat << 'EOF' > tests_input/input_task04_err_zero_size.txt
2 50
file1.txt 0 10
file2.txt 10 20
EOF

# Отрицательная важность файла
cat << 'EOF' > tests_input/input_task04_err_neg_imp.txt
2 50
file1.txt 10 -5
file2.txt 10 20
EOF

# Дублирующиеся имена файлов (по условию имена уникальны)
cat << 'EOF' > tests_input/input_task04_err_dup_names.txt
2 50
same_name.txt 10 10
same_name.txt 20 20
EOF

# Неполные данные (указано n=3, а строк всего 2)
cat << 'EOF' > tests_input/input_task04_err_missing_line.txt
3 50
file1.txt 10 10
file2.txt 20 20
EOF

# Лишние данные (лишний столбец)
cat << 'EOF' > tests_input/input_task04_err_extra_column.txt
2 50
file1.txt 10 10 EXTRA_ARG
file2.txt 20 20
EOF

# Огромное число (переполнение)
cat << 'EOF' > tests_input/input_task04_err_overflow.txt
999999999999999999999999999999999999 50
file1.txt 10 10
EOF

echo "Тестовые входные файлы для задания 4 созданы в папке tests_input/"