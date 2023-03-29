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
    antlr4 -Dlanguage=Python3 source/grammar/Slap.g4
    mkdir -p source/parsing
    mv source/grammar/*.py source/parsing
    touch source/parsing/__init__.py
    find . -name "*.interp" -type f -delete
    find . -name "*.tokens" -type f -delete
}

Clean() {
    echo "Cleaning Slap..."
    rm -r bin
    rm -r source/parsing
}

# Parse the command line arguments
echo "From build.sh"
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
            Executeable ${@:2}
            exit 0
            ;;
        s)
            Script ${@:2}
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




