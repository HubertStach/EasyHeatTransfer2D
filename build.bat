@echo off
setlocal

:: Zmienne konfiguracyjne
set BUILD_DIR=build
set BUILD_TYPE=Debug
set PROJECT_NAME=EasyHeatTransfer2D

echo ==========================================
echo  Building project %PROJECT_NAME% (%BUILD_TYPE%) 
echo ==========================================

echo [1/2] Configuring CMake...
cmake -S . -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -G "MinGW Makefiles"
if %errorlevel% neq 0 (
    echo.
    echo [BLAD] Konfiguracja CMake nie powiodla sie!
    echo Sprawdz, czy folder bin z MSYS2 znajduje sie w zmiennej PATH.
    pause
    exit /b %errorlevel%
)

:: 2. Wlasciwe budowanie (kompilacja)
echo [2/2] Compilating via CMake...
cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel
if %errorlevel% neq 0 (
    echo.
    echo [BLAD] Compilation wwasn't succesfull!
    pause
    exit /b %errorlevel%
)

echo ==========================================
echo  Project has been built.
echo  To run type ./build/EHT2D.exe
echo ==========================================

build\EHT2D.exe