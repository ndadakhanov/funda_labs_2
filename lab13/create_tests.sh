#!/bin/bash

mkdir -p tests_data
mkdir -p tests_args

# 1. Тестовые файлы с текстом
cat << 'EOF' > tests_data/file1.txt
hello world
this is hello again
and hello one more time
EOF

cat << 'EOF' > tests_data/file2_empty.txt
EOF

cat << 'EOF' > tests_data/file3_no_match.txt
just some random text
without the word you want
EOF

cat << 'EOF' > tests_data/file4_multiline.txt
start here
and continue here
end of file
EOF

cat << 'EOF' > tests_data/file5_tabs_spaces.txt
	hello	tabbed
  hello   spaced
hello
EOF

# 2. Файлы аргументов (аргументы экранируются кавычками при необходимости)
echo 'hello tests_data/file1.txt tests_data/file3_no_match.txt' > tests_args/args_task03_multi_files.txt
echo 'hello tests_data/file1.txt' > tests_args/args_task03_single_file.txt
echo 'notfound tests_data/file1.txt' > tests_args/args_task03_no_matches.txt
echo 'here\nand tests_data/file4_multiline.txt' > tests_args/args_task03_multiline.txt
echo 'hello tests_data/file5_tabs_spaces.txt' > tests_args/args_task03_tabs_spaces.txt
echo 'hello tests_data/non_existent_file.txt tests_data/file1.txt' > tests_args/args_task03_non_existent.txt

# Ошибки
echo '"" tests_data/file1.txt' > tests_args/args_task03_err_empty_substr.txt
echo 'hello' > tests_args/args_task03_err_no_files.txt

echo "Тестовые данные для лабораторной 3 успешно созданы!"