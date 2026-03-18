@echo off
setlocal

:: Zmienne konfiguracyjne
set BUILD_DIR=build
set BUILD_TYPE=Debug
set PROJECT_NAME=easyFEM

echo ==========================================
echo  Budowanie projektu %PROJECT_NAME% (%BUILD_TYPE%) 
echo ==========================================

:: 1. Konfiguracja
echo [1/2] Konfiguracja CMake...
cmake -S . -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -G "MinGW Makefiles"
if %errorlevel% neq 0 (
    echo.
    echo [BLAD] Konfiguracja CMake nie powiodla sie!
    echo Sprawdz, czy folder bin z MSYS2 znajduje sie w zmiennej PATH.
    pause
    exit /b %errorlevel%
)

:: 2. Wlasciwe budowanie (kompilacja)
echo [2/2] Kompilacja przez CMake...
cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel
if %errorlevel% neq 0 (
    echo.
    echo [BLAD] Kompilacja nie powiodla sie!
    pause
    exit /b %errorlevel%
)

echo ==========================================
echo  Sukces! Projekt zostal zbudowany.
echo  Plik wykonywalny znajdziesz w folderze: %BUILD_DIR%
echo ==========================================

:: Zatrzymuje zamykanie okna
pause
