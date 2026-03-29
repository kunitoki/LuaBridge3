default:
    @just -l

generate:
    cmake -G Xcode -B Build .

clean:
    rm -rf Build

benchmark:
    cmake -G Xcode -B Build -DLUABRIDGE_BENCHMARKS=ON .
    cmake --build Build --config Release --target LuaBridgeBenchmarks -j8
    ./Build/Benchmarks/Release/LuaBridgeBenchmarks 1000000

amalgamate:
    python3 amalgamate.py
