#!/bin/bash

# Zatrzymuje działanie skryptu, jeśli jakakolwiek komenda zwróci błąd
set -e

# Zmienne konfiguracyjne
BUILD_DIR="build"
BUILD_TYPE="Debug"
PROJECT_NAME="easyFEM"

echo "=========================================="
echo " Budowanie projektu $PROJECT_NAME ($BUILD_TYPE) "
echo "=========================================="

# 1. Konfiguracja (generowanie plików budowania)
echo "[1/2] Konfiguracja CMake..."
# Tworzy folder build/ i konfiguruje projekt.
# CMake sam dobierze najlepszy domyślny system budowania dla systemu użytkownika.
cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# 2. Właściwe budowanie (kompilacja)
echo "[2/2] Kompilacja przez CMake..."
# Uniwersalna komenda kompilacji - ukrywa to czy pod spodem jest Make, Ninja czy coś innego.
# --parallel sprawia, że kompilacja użyje wielu rdzeni procesora, co ją znacznie przyspieszy.
cmake --build $BUILD_DIR --config $BUILD_TYPE --parallel

echo "=========================================="
echo " Sukces! Projekt został zbudowany."
echo " Aby go uruchomić, wpisz w terminalu:"
echo " ./$BUILD_DIR/$PROJECT_NAME"
echo "=========================================="