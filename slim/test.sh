#!/usr/bin/bash

./auto.sh -b
echo "Assembling test.slap"
cd ../slap && python3 source/Slap.py ../test.slap ../test.slim && cd ../slim
echo "Running test.slim"
./bin/exe ../test.slim