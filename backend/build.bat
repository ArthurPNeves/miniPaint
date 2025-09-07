@echo off
setlocal enabledelayedexpansion

REM Cores para output (limitado no Windows)
set "INFO=echo [INFO]"
set "SUCCESS=echo [SUCCESS]"
set "ERROR=echo [ERROR]"
set "WARNING=echo [WARNING]"

REM Nome do projeto
for %%I in (.) do set PROJECT=%%~nxI

REM Verificar argumentos
if "%1"=="" (
    %ERROR% Nenhum target especificado!
    call :show_help
    exit /b 1
)

if "%1"=="help" call :show_help && exit /b 0
if "%1"=="-h" call :show_help && exit /b 0
if "%1"=="--help" call :show_help && exit /b 0

REM Executar o target
if "%1"=="windows" call :build_windows
if "%1"=="windows-simple" call :build_windows_simple
if "%1"=="clean" call :clean_build
if "%1"=="linux" (
    %ERROR% Target Linux nao suportado no Windows. Use WSL ou cross-compilation.
    exit /b 1
)

REM Se chegou aqui, target desconhecido
%ERROR% Target desconhecido: %1
call :show_help
exit /b 1

:build_windows
%INFO% Compilando para Windows com servidor...
call :check_mingw
if errorlevel 1 exit /b 1

if exist build rmdir /s /q build
if exist bin rmdir /s /q bin

mkdir build 2>nul
mkdir bin 2>nul

%INFO% Compilando arquivos fonte...
g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\algorithms.o algorithms.cpp
if errorlevel 1 (%ERROR% Falha ao compilar algorithms.cpp && exit /b 1)

g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\transformations.o transformations.cpp
if errorlevel 1 (%ERROR% Falha ao compilar transformations.cpp && exit /b 1)

g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\server.o server.cpp
if errorlevel 1 (%ERROR% Falha ao compilar server.cpp && exit /b 1)

%INFO% Linkando executavel...
g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -o bin\%PROJECT%.exe build\algorithms.o build\transformations.o build\server.o -lws2_32 -lwsock32
if errorlevel 1 (%ERROR% Falha no linking && exit /b 1)

%SUCCESS% Build Windows completo! Executavel: bin\%PROJECT%.exe
goto :eof

:build_windows_simple
%INFO% Compilando versao simples para Windows...
call :check_mingw
if errorlevel 1 exit /b 1

if exist build rmdir /s /q build
if exist bin rmdir /s /q bin

mkdir build 2>nul
mkdir bin 2>nul

%INFO% Compilando arquivos fonte...
g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\algorithms.o algorithms.cpp
if errorlevel 1 (%ERROR% Falha ao compilar algorithms.cpp && exit /b 1)

g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\transformations.o transformations.cpp
if errorlevel 1 (%ERROR% Falha ao compilar transformations.cpp && exit /b 1)

g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\main_simple.o main_simple.cpp
if errorlevel 1 (%ERROR% Falha ao compilar main_simple.cpp && exit /b 1)

%INFO% Linkando executavel...
g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -o bin\%PROJECT%-simple.exe build\algorithms.o build\transformations.o build\main_simple.o
if errorlevel 1 (%ERROR% Falha no linking && exit /b 1)

%SUCCESS% Build Windows simples completo! Executavel: bin\%PROJECT%-simple.exe
goto :eof

:clean_build
%INFO% Limpando artefatos de build...
if exist build rmdir /s /q build
if exist bin rmdir /s /q bin
%SUCCESS% Limpeza concluida!
goto :eof

:check_mingw
where g++ >nul 2>&1
if errorlevel 1 (
    %ERROR% g++ nao encontrado. Instale MinGW-w64 ou MSYS2
    %ERROR% Download: https://www.mingw-w64.org/downloads/
    exit /b 1
)
goto :eof

:show_help
echo.
echo Uso: %0 {windows^|windows-simple^|clean^|help}
echo.
echo Targets de build:
echo   windows         - Build para Windows com servidor
echo   windows-simple  - Build versao simples para Windows
echo   clean           - Limpar artefatos de build
echo   help            - Mostrar esta ajuda
echo.
echo Exemplos:
echo   %0 windows
echo   %0 windows-simple
echo.
echo Nota: Para compilar no Windows, voce precisa ter MinGW-w64 instalado
echo       e o g++ disponivel no PATH do sistema.
goto :eof
