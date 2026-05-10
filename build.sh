#!/bin/bash

# Zatrzymuje działanie skryptu, jeśli jakakolwiek komenda zwróci błąd
set -e

# Zmienne konfiguracyjne
BUILD_DIR="build"
BUILD_TYPE="Debug"
PROJECT_NAME="EasyHeatTransfer2D" # Nazwa tylko do wyświetlania w terminalu

echo "=========================================="
echo " Building project $PROJECT_NAME ($BUILD_TYPE) "
echo "=========================================="

# 0. Automatyczna instalacja zależności systemowych (Ubuntu / Debian / Mint / WSL)
echo "[0/2] Checking system dependencies..."

# Sprawdzamy, czy system używa menedżera pakietów 'apt'
if command -v apt-get >/dev/null; then
    # Sprawdzamy czy brakuje g++ LUB biblioteki libx11-dev
    if ! command -v g++ >/dev/null || ! dpkg -s libx11-dev >/dev/null 2>&1; then
        echo "Missing required system packages. Trying to install them..."
        echo "You might be asked for your sudo password."
        sudo apt-get update
        # Flaga -y sprawia, że apt nie będzie pytał "Czy na pewno chcesz zainstalować? [T/n]"
        sudo apt-get install -y build-essential cmake libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev libgl1-mesa-dev libglx-dev
        echo "Dependencies installed successfully!"
    else
        echo "All system dependencies are already installed. Skipping."
    fi
else
    echo "Warning: 'apt' package manager not found (you are likely not on Ubuntu/Debian)."
    echo "Please ensure you have C++ compiler, CMake and X11/OpenGL dev packages installed."
fi

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
