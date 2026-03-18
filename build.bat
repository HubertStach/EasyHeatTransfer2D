@echo off
setlocal

set BUILD_DIR=build
set BUILD_TYPE=Debug
set PROJECT_NAME=easyFEM

echo ==========================================
echo  Building project %PROJECT_NAME% (%BUILD_TYPE%)
echo ==========================================

:: 1. Konfiguracja (generowanie plikow budowania)
echo [1/2] Configuring CMake...
cmake -S . -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if %errorlevel% neq 0 exit /b %errorlevel%

:: 2. Wlasciwe budowanie (kompilacja)
echo [2/2] Compilating using CMake...
cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel
if %errorlevel% neq 0 exit /b %errorlevel%

echo ==========================================
echo  Project has been built succesfully
echo  to run %PROJECT_NAME%.exe write:
echo  .\%BUILD_DIR%\%BUILD_TYPE%\%PROJECT_NAME%.exe
echo ==========================================

pause