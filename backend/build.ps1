# Build script para Windows PowerShell
param(
    [Parameter(Mandatory=$true)]
    [ValidateSet("windows", "windows-simple", "clean", "help")]
    [string]$Target
)

# Cores para output
function Write-Info($Message) {
    Write-Host "[INFO] $Message" -ForegroundColor Blue
}

function Write-Success($Message) {
    Write-Host "[SUCCESS] $Message" -ForegroundColor Green
}

function Write-Error($Message) {
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

function Write-Warning($Message) {
    Write-Host "[WARNING] $Message" -ForegroundColor Yellow
}

# Nome do projeto
$PROJECT = Split-Path -Leaf (Get-Location)

function Test-MinGW {
    try {
        $null = Get-Command g++ -ErrorAction Stop
        return $true
    }
    catch {
        Write-Error "g++ não encontrado. Instale MinGW-w64 ou MSYS2"
        Write-Error "Download: https://www.mingw-w64.org/downloads/"
        return $false
    }
}

function Build-Windows {
    Write-Info "Compilando para Windows com servidor..."
    
    if (-not (Test-MinGW)) { return }
    
    # Limpar diretórios
    if (Test-Path "build") { Remove-Item -Recurse -Force "build" }
    if (Test-Path "bin") { Remove-Item -Recurse -Force "bin" }
    
    # Criar diretórios
    New-Item -ItemType Directory -Force "build" | Out-Null
    New-Item -ItemType Directory -Force "bin" | Out-Null
    
    Write-Info "Compilando arquivos fonte..."
    
    # Compilar
    & g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\algorithms.o algorithms.cpp
    if ($LASTEXITCODE -ne 0) { Write-Error "Falha ao compilar algorithms.cpp"; return }
    
    & g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\transformations.o transformations.cpp
    if ($LASTEXITCODE -ne 0) { Write-Error "Falha ao compilar transformations.cpp"; return }
    
    & g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\server.o server.cpp
    if ($LASTEXITCODE -ne 0) { Write-Error "Falha ao compilar server.cpp"; return }
    
    Write-Info "Linkando executável..."
    & g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -o bin\$PROJECT.exe build\algorithms.o build\transformations.o build\server.o -lws2_32 -lwsock32
    if ($LASTEXITCODE -ne 0) { Write-Error "Falha no linking"; return }
    
    Write-Success "Build Windows completo! Executável: bin\$PROJECT.exe"
}

function Build-WindowsSimple {
    Write-Info "Compilando versão simples para Windows..."
    
    if (-not (Test-MinGW)) { return }
    
    # Limpar diretórios
    if (Test-Path "build") { Remove-Item -Recurse -Force "build" }
    if (Test-Path "bin") { Remove-Item -Recurse -Force "bin" }
    
    # Criar diretórios
    New-Item -ItemType Directory -Force "build" | Out-Null
    New-Item -ItemType Directory -Force "bin" | Out-Null
    
    Write-Info "Compilando arquivos fonte..."
    
    # Compilar
    & g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\algorithms.o algorithms.cpp
    if ($LASTEXITCODE -ne 0) { Write-Error "Falha ao compilar algorithms.cpp"; return }
    
    & g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\transformations.o transformations.cpp
    if ($LASTEXITCODE -ne 0) { Write-Error "Falha ao compilar transformations.cpp"; return }
    
    & g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -c -o build\main_simple.o main_simple.cpp
    if ($LASTEXITCODE -ne 0) { Write-Error "Falha ao compilar main_simple.cpp"; return }
    
    Write-Info "Linkando executável..."
    & g++ -std=c++17 -O2 -Wall -Wextra -Ilibs -o bin\$PROJECT-simple.exe build\algorithms.o build\transformations.o build\main_simple.o
    if ($LASTEXITCODE -ne 0) { Write-Error "Falha no linking"; return }
    
    Write-Success "Build Windows simples completo! Executável: bin\$PROJECT-simple.exe"
}

function Clean-Build {
    Write-Info "Limpando artefatos de build..."
    if (Test-Path "build") { Remove-Item -Recurse -Force "build" }
    if (Test-Path "bin") { Remove-Item -Recurse -Force "bin" }
    Write-Success "Limpeza concluída!"
}

function Show-Help {
    Write-Host ""
    Write-Host "Uso: .\build.ps1 -Target <target>"
    Write-Host ""
    Write-Host "Targets de build:"
    Write-Host "  windows         - Build para Windows com servidor"
    Write-Host "  windows-simple  - Build versão simples para Windows"
    Write-Host "  clean           - Limpar artefatos de build"
    Write-Host "  help            - Mostrar esta ajuda"
    Write-Host ""
    Write-Host "Exemplos:"
    Write-Host "  .\build.ps1 -Target windows"
    Write-Host "  .\build.ps1 -Target windows-simple"
    Write-Host ""
    Write-Host "Nota: Para compilar no Windows, você precisa ter MinGW-w64 instalado"
    Write-Host "      e o g++ disponível no PATH do sistema."
}

# Executar o target
switch ($Target) {
    "windows" { Build-Windows }
    "windows-simple" { Build-WindowsSimple }
    "clean" { Clean-Build }
    "help" { Show-Help }
}
