#!/usr/bin/bash

./auto.sh -b
echo "Assembling test.slap"
cd ../slap && python3 source/Slap.py ../test.slap && cd ../slim
echo "Running test.slim"
if [ -f test.log ]; then
    rm test.log
fi
./bin/exe test.log ../test.slim