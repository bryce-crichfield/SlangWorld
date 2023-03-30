#!/usr/bin/bash

FILES=$(find . -type f -name "*.c")
clang-format -i -style=file $FILES