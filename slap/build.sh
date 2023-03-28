#!/usr/bin/bash

Help() {
    echo "-h : Display this help message"
    echo "-b : Build the project into an executable"
    echo "-e : Run the project as an executable"
    echo "-s : Run the project as a script"
    echo "-t : Type check the project"
    echo "-g : Compile the grammar into a parser"
    echo "-c : Clean the project"
}

Build() {
    echo "Building Slap executable..."
    pyinstaller -F --distpath ./bin source/Slap.py
    rm -r build
    mv bin/Slap bin/exe
}

Executeable() {
    echo "Running Slap executable..."
    ./bin/exe $@
}

Script() {
    echo "Running Slap script..."
    python3 source/Slap.py $@
}

Typecheck() {
    echo "Type checking Slap..."
    mypy source
}

Grammar() {
    echo "Compiling the Slap grammar..."
    antlr4 -Dlanguage=Python3 -no-visitor source/grammar/Slap.g4 -o source/parsing

    # Remove the intermediate files ending in ".interp" or ".token"
    pushd .
    cd source/parsing
    rm -f *.interp
    rm -f *.tokens
    popd
}

Clean() {
    echo "Cleaning Slap..."
    rm -r bin
    rm -r source/parsing
}

# Parse the command line arguments
while getopts "hbesgtc" opt; do
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
        s)
            Script
            exit 0
            ;;
        t)
            Typecheck
            exit 0
            ;;
        g)
            Grammar
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




