#!/bin/bash

# Zatrzymuje działanie skryptu, jeśli jakakolwiek komenda zwróci błąd
set -e

# Zmienne konfiguracyjne
BUILD_DIR="build"
BUILD_TYPE="Debug"
PROJECT_NAME="easyFEM" # Ta zmienna służy tylko do wyświetlania w skrypcie

echo "=========================================="
echo " Building project $PROJECT_NAME ($BUILD_TYPE) "
echo "=========================================="

# 1. Konfiguracja (generowanie plików budowania)
echo "[1/2] Configuring CMake..."
cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# 2. Właściwe budowanie (kompilacja)
echo "[2/2] Compiling using CMake..."
cmake --build $BUILD_DIR --config $BUILD_TYPE --parallel

echo "=========================================="
echo "Project has been built successfully!"
echo "To run the application, type:"
echo "./$BUILD_DIR/EHT2D"
echo "=========================================="
