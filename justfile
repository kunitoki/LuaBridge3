
default:
    @just -l

generate:
    cmake -G Xcode -B Build .

sanitize TYPE='address':
    cmake -G Xcode -B Build -DLUABRIDGE_SANITIZE={{TYPE}} .

benchmark:
    cmake -G Xcode -B Build -DLUABRIDGE_BENCHMARKS=ON .
    cmake --build Build --config Release --target LuaBridge3Benchmark -j8
    cmake --build Build --config Release --target LuaBridgeVanillaBenchmark -j8
    cmake --build Build --config Release --target Sol3Benchmark -j8
    ./Build/Benchmarks/Release/LuaBridge3Benchmark --benchmark_out_format=json --benchmark_out=Build/LuaBridge3Benchmark.json
    ./Build/Benchmarks/Release/LuaBridgeVanillaBenchmark --benchmark_out_format=json --benchmark_out=Build/LuaBridgeVanillaBenchmark.json
    ./Build/Benchmarks/Release/Sol3Benchmark --benchmark_out_format=json --benchmark_out=Build/Sol3Benchmark.json
    @just plot

plot:
    python3 Benchmarks/plot_benchmarks.py --input Build/LuaBridge3Benchmark.json Build/LuaBridgeVanillaBenchmark.json Build/Sol3Benchmark.json --output Images/benchmarks.png

clean:
    rm -rf Build

amalgamate:
    python3 amalgamate.py
