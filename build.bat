@echo off
setlocal

:: Zmienne konfiguracyjne
set BUILD_DIR=build
set BUILD_TYPE=Debug
set PROJECT_NAME=easyFEM

echo ==========================================
echo  Budowanie projektu %PROJECT_NAME% (%BUILD_TYPE%)
echo ==========================================

:: 1. Konfiguracja (generowanie plikow budowania)
echo [1/2] Konfiguracja CMake...
cmake -S . -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if %errorlevel% neq 0 exit /b %errorlevel%

:: 2. Wlasciwe budowanie (kompilacja)
echo [2/2] Kompilacja przez CMake...
cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel
if %errorlevel% neq 0 exit /b %errorlevel%

echo ==========================================
echo  Sukces! Projekt zostal zbudowany.
echo  Aby go uruchomic, wejdz do folderu %BUILD_DIR%
echo  i uruchom %PROJECT_NAME%.exe lub wpisz w terminalu:
echo  .\%BUILD_DIR%\%BUILD_TYPE%\%PROJECT_NAME%.exe
echo ==========================================

:: Zatrzymuje zamykanie okna, jesli uzytkownik kliknal plik 2 razy
pause