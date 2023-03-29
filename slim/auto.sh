#!/usr/bin/bash

Help() {
    echo "SLIM Build Functions"
    echo "-h : Display this help message"
    echo "-b : Build the executable"
    echo "-e : Run the executable"
    echo "-c : Clean the build directory"
}

Build() {
    echo "Building Slim..."
    SOURCES=$(find . -name "*.c")
    LIBS="-lm"
    CFLAGS="-Wall -Werror -O3"

    mkdir -p bin
    clang $SOURCES -o bin/exe $LIBS $CFLAGS
}

Executeable() {
    echo "Running Slim..."
    ./bin/exe $@
}

Clean() {
    echo "Cleaning Slim..."
    rm -r bin
}

# Parse the command line arguments
while getopts "hbec" opt; do
    case $opt in
        h)
            Help
            exit 0
            ;;
        b)
            Build
            exit 0
            ;;
        e)
            Executeable
            exit 0
            ;;
        c)
            Clean
            exit 0
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            exit 1
            ;;
    esac
done