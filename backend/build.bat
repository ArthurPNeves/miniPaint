@echo off
setlocal enabledelayedexpansion

REM Nome do projeto
for %%I in (.) do set PROJECT=%%~nxI

REM Verificar argumentos
if "%1"=="" (
    echo [ERROR] Nenhum target especificado!
    goto show_help
)

if "%1"=="help" goto show_help
if "%1"=="-h" goto show_help
if "%1"=="--help" goto show_help
if "%1"=="clean" goto clean_build

if not "%1"=="windows" (
    if "%1"=="linux" (
        echo [ERROR] Target Linux nao suportado no Windows. Use WSL ou cross-compilation.
        exit /b 1
    ) else (
        echo [ERROR] Target desconhecido: %1
        goto show_help
    )
)

REM Se chegou aqui, é o target windows
goto build_windows

:build_windows
echo [INFO] Compilando para Windows com servidor...
call :check_mingw
if errorlevel 1 exit /b 1

if exist build rmdir /s /q build 2>nul
if exist bin rmdir /s /q bin 2>nul

mkdir build 2>nul
mkdir bin 2>nul

echo [INFO] Compilando arquivos fonte...

REM Verificar se os arquivos fonte existem
if not exist algorithms.cpp (
    echo [ERROR] Arquivo algorithms.cpp nao encontrado!
    exit /b 1
)
if not exist transformations.cpp (
    echo [ERROR] Arquivo transformations.cpp nao encontrado!
    exit /b 1
)
if not exist server.cpp (
    echo [ERROR] Arquivo server.cpp nao encontrado!
    exit /b 1
)
g++ -D_WIN32_WINNT=0x0A00 -std=c++17 -o output.exe source.cpp

echo [INFO] Compilando algorithms.cpp...
g++ -D_WIN32_WINNT=0x0A00 -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\algorithms.o algorithms.cpp
if errorlevel 1 (
    echo [ERROR] Falha ao compilar algorithms.cpp
    exit /b 1
)

echo [INFO] Compilando transformations.cpp...
g++ -D_WIN32_WINNT=0x0A00 -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\transformations.o transformations.cpp
if errorlevel 1 (
    echo [ERROR] Falha ao compilar transformations.cpp
    exit /b 1
)

echo [INFO] Compilando server.cpp...
g++ -D_WIN32_WINNT=0x0A00 -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\server.o server.cpp
if errorlevel 1 (
    echo [ERROR] Falha ao compilar server.cpp
    exit /b 1
)

echo [INFO] Linkando executavel...
g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -o bin\%PROJECT%.exe build\algorithms.o build\transformations.o build\server.o -lws2_32 -lwsock32
if errorlevel 1 (
    echo [ERROR] Falha no linking
    exit /b 1
)

echo [SUCCESS] Build Windows completo! Executavel: bin\%PROJECT%.exe
exit /b 0

:clean_build
echo [INFO] Limpando artefatos de build...
if exist build rmdir /s /q build 2>nul
if exist bin rmdir /s /q bin 2>nul
echo [SUCCESS] Limpeza concluida!
exit /b 0

:check_mingw
where g++ >nul 2>&1
if errorlevel 1 (
    echo [ERROR] g++ nao encontrado. Instale MinGW-w64
    exit /b 1
)

echo [INFO] MinGW-w64 encontrado: 
g++ --version | findstr "g++"
exit /b 0

:show_help
echo.
echo Uso: %~nx0 {windows^|clean^|help}
echo.
echo Targets de build:
echo   windows         - Build para Windows com servidor
echo   clean           - Limpar artefatos de build
echo   help            - Mostrar esta ajuda
echo.
echo Exemplos:
echo   %~nx0 windows
echo   %~nx0 clean
echo.
echo Nota: Para compilar no Windows, voce precisa ter MinGW-w64 instalado
echo       e o g++ disponivel no PATH do sistema.
echo.
exit /b 0