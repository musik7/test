#!/bin/bash
# run_demo.sh - Build & test semua dalam 1 command

set -e  # Stop kalau ada error

echo "========================================"
echo "🧪 STEP 1: Generate Flatbuffers header"
echo "========================================"
flatc --cpp -o include/ schema/profile.fbs

echo ""
echo "========================================"
echo "🛠️  STEP 2: Build Native (CLI + Tests)"
echo "========================================"
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

echo ""
echo "========================================"
echo "🧪 STEP 3: Running Unit Tests"
echo "========================================"
./run_tests

echo ""
echo "========================================"
echo "📊 STEP 4: Generate dummy profile & test CLI"
echo "========================================"
./cli_decoder --generate-dummy --output sample.bin
./cli_decoder --input sample.bin

echo ""
echo "========================================"
echo "🌐 STEP 5: Build Wasm (Emscripten)"
echo "========================================"
cd ..
mkdir -p build_wasm && cd build_wasm
emcmake cmake .. -DBUILD_WASM=ON
make -j$(nproc)

echo ""
echo "========================================"
echo "✅ ALL DONE! Wasm ada di:"
ls -la agent.*
echo "========================================"
