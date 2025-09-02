#!/usr/bin/env bash

echo "= BUILD BENCHMARK"

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

echo "= RUN BENCHMARK"

cd build/frxml-test || exit 1
./frxml-test --benchmark_min_time=2s --benchmark_format=json > results.json
cd ../..

echo "= VISUALIZE BENCHMARK"

cp -r frxml-test/graph build/graph
cp build/frxml-test/results.json build/graph

cd build/graph || exit 1
python -m venv .venv
.venv/bin/pip install -r requirements.txt
.venv/bin/python main.py
cd ../..

echo "= DONE"
