#!/bin/bash

# Путь к архиву
ARCHIVE_PATH="ARCHIVE_PATH"
# Путь к программе распаковки
EXTRACTOR_PATH="EXTRACTOR_PATH"

# Создаем временную директорию
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

# Копируем программу распаковки
cp "$EXTRACTOR_PATH" ./extractor
chmod +x ./extractor

# Распаковываем архив
./extractor "$ARCHIVE_PATH" "$TEMP_DIR"

# Запускаем распакованные файлы
if [ -f "autorun.sh" ]; then
    chmod +x autorun.sh
    ./autorun.sh
fi

# Очищаем временные файлы
rm -rf "$TEMP_DIR" 