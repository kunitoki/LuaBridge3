
default:
    @just -l

generate:
    cmake -G Xcode -B Build -DLUABRIDGE_BENCHMARKS=ON .

sanitize TYPE='address':
    cmake -G Xcode -B Build -DLUABRIDGE_SANITIZE={{TYPE}} .

benchmark:
    @just generate
    cmake --build Build --config Release --target LuaBridge3Benchmark -j8
    cmake --build Build --config Release --target LuaBridgeVanillaBenchmark -j8
    cmake --build Build --config Release --target Sol3Benchmark -j8
    ./Build/Benchmarks/Release/LuaBridge3Benchmark --benchmark_out_format=json --benchmark_out=Build/LuaBridge3Benchmark.json
    ./Build/Benchmarks/Release/LuaBridgeVanillaBenchmark --benchmark_out_format=json --benchmark_out=Build/LuaBridgeVanillaBenchmark.json
    ./Build/Benchmarks/Release/Sol3Benchmark --benchmark_out_format=json --benchmark_out=Build/Sol3Benchmark.json
    @just plot

plot:
    uv run --with-requirements Benchmarks/requirements.txt Benchmarks/plot_benchmarks.py --input Build/*.json --output Images/benchmarks.png

clean:
    -rm -rf Build/
    rm -rf Build/

amalgamate:
    uv run Tools/amalgamate.py
