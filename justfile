
default:
    @just -l

generate CXX="17":
    cmake -G Xcode -B Build{{CXX}} -DLUABRIDGE_BENCHMARKS=ON -DCMAKE_CXX_STANDARD={{CXX}} .

open CXX="17":
    @just generate {{CXX}}
    -open Build{{CXX}}/LuaBridge.xcodeproj

build CXX="17":
    @just generate {{CXX}}
    cmake --build Build{{CXX}} --config Debug -j8

test CXX="17":
    @just build {{CXX}}
    ctest --test-dir Build{{CXX}} -C Debug -j8

test-all:
    @just test 17
    @just test 20
    @just test 23

build1 CXX="17":
    @just generate {{CXX}}
    cmake --build Build{{CXX}} --config Debug --target LuaBridgeTests54 -j8

test1 CXX="17":
    @just build1 {{CXX}}
    ./Build{{CXX}}/Tests/Debug/LuaBridgeTests54

sanitize TYPE="address" CXX="17":
    cmake -G Xcode -B Build{{CXX}} -DLUABRIDGE_SANITIZE={{TYPE}} .

gcc CXX="17":
    cmake -G "Unix Makefiles" \
        -DCMAKE_C_COMPILER=/opt/homebrew/bin/gcc-15 \
        -DCMAKE_CXX_COMPILER=/opt/homebrew/bin/g++-15 \
        -B BuildGCC{{CXX}} -DLUABRIDGE_BENCHMARKS=ON -DCMAKE_CXX_STANDARD={{CXX}} .
    cmake --build BuildGCC{{CXX}} --config Debug --target LuaBridgeTests54 -j8
    ./BuildGCC{{CXX}}/Tests/LuaBridgeTests54 --gtest_filter=*

benchmark CXX="17":
    @just generate {{CXX}}
    cmake --build Build{{CXX}} --config Release --target LuaBridge3Benchmark -j8
    cmake --build Build{{CXX}} --config Release --target LuaBridgeVanillaBenchmark -j8
    cmake --build Build{{CXX}} --config Release --target Sol3Benchmark -j8
    ./Build{{CXX}}/Benchmarks/Release/LuaBridge3Benchmark --benchmark_out_format=json --benchmark_out=Build{{CXX}}/LuaBridge3Benchmark.json
    ./Build{{CXX}}/Benchmarks/Release/LuaBridgeVanillaBenchmark --benchmark_out_format=json --benchmark_out=Build{{CXX}}/LuaBridgeVanillaBenchmark.json
    ./Build{{CXX}}/Benchmarks/Release/Sol3Benchmark --benchmark_out_format=json --benchmark_out=Build{{CXX}}/Sol3Benchmark.json
    @just plot {{CXX}}

plot CXX="17":
    uv run --with-requirements Benchmarks/requirements.txt Benchmarks/plot_benchmarks.py --input Build{{CXX}}/*.json --output Images/benchmarks.png

amalgamate:
    uv run amalgamate.py

docs:
    uv run --with-requirements docs/requirements.txt sphinx-build -W -b html docs docs/_build/html

docs-open:
    @just docs
    open docs/_build/html/index.html

clean:
    rm -rf Build

