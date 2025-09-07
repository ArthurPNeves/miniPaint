#!/bin/bash

set -e

PROJECT=$(basename "$PWD")

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

echo_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

echo_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_dependencies() {
    case "$1" in
        "windows")
            if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
                echo_error "MinGW-w64 not found. Install with: sudo apt install mingw-w64"
                exit 1
            fi
            ;;
    esac
}

show_help() {
    echo "Usage: $0 {linux|windows|windows-simple|clean|help}"
    echo ""
    echo "Build targets:"
    echo "  linux              - Build for Linux (native)"
    echo "  windows            - Build for Windows with server (requires mingw-w64)"
    echo "  windows-simple     - Build simple version for Windows (requires mingw-w64)"
    echo "  clean              - Clean build artifacts"
    echo "  help               - Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 linux"
    echo "  $0 windows-simple"
}

case "$1" in
    "linux")
        echo_info "Building for Linux..."
        check_dependencies "$1"
        make clean && make linux
        echo_success "Linux build completed! Binary: bin/$PROJECT"
        ;;
    "windows")
        echo_info "Building for Windows with server..."
        check_dependencies "$1"
        make clean && make windows
        echo_success "Windows build completed! Binary: bin/$PROJECT.exe"
        ;;
    "clean")
        echo_info "Cleaning build artifacts..."
        make clean
        echo_success "Clean completed!"
        ;;
    "help"|"-h"|"--help")
        show_help
        ;;
    "")
        echo_error "No target specified!"
        show_help
        exit 1
        ;;
    *)
        echo_error "Unknown target: $1"
        show_help
        exit 1
        ;;
esac