default:
    @just -l

generate:
    cmake -G Xcode -B Build .

sanitizer TYPE='address':
    cmake -G Xcode -B Build -DLUABRIDGE_SANITIZE={{TYPE}} .

benchmark:
    cmake -G Xcode -B Build -DLUABRIDGE_BENCHMARKS=ON .
    cmake --build Build --config Release --target LuaBridgeBenchmarks -j8
    ./Build/Benchmarks/Release/LuaBridgeBenchmarks 1000000

clean:
    rm -rf Build

amalgamate:
    python3 amalgamate.py
