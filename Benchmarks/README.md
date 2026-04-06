# Lua Binding Benchmarks

This directory contains Google Benchmark based executables for:

- LuaBridge3 (current workspace)
- LuaBridge vanilla (`https://github.com/vinniefalco/LuaBridge`)
- sol3 (`https://github.com/ThePhD/sol2`)

All benchmark executables are built with the same embedded Lua 5.4.8 runtime source (`Tests/Lua/LuaLibrary5.4.8.cpp`) for fair comparisons.

## Build

From project root:

```bash
cmake -S . -B Build -DCMAKE_BUILD_TYPE=Release -DLUABRIDGE_BENCHMARKS=ON
cmake --build Build --config Release --target LuaBridge3Benchmark LuaBridgeVanillaBenchmark
```

To also build Sol3 benchmark target:

```bash
cmake -S . -B Build -DCMAKE_BUILD_TYPE=Release -DLUABRIDGE_BENCHMARKS=ON -DLUABRIDGE_BENCHMARK_WITH_SOL3=ON
cmake --build Build --config Release --target Sol3Benchmark
```

## Dependency Sources (FetchContent)

Defaults:

- Google Benchmark: `https://github.com/google/benchmark.git` (`v1.8.4`)
- sol3: `https://github.com/ThePhD/sol2.git` (`v3.5.0`)
- LuaBridge vanilla: `https://github.com/vinniefalco/LuaBridge.git` (`master`)

You can override these at configure time:

```bash
cmake -S . -B Build \
  -DCMAKE_BUILD_TYPE=Release \
  -DLUABRIDGE_BENCHMARKS=ON \
  -DLUABRIDGE_SOL2_GIT_REPOSITORY=https://github.com/ThePhD/sol2.git \
  -DLUABRIDGE_SOL2_GIT_TAG=v3.5.0 \
  -DLUABRIDGE_VANILLA_GIT_REPOSITORY=https://github.com/vinniefalco/LuaBridge.git \
  -DLUABRIDGE_VANILLA_GIT_TAG=master \
  -DLUABRIDGE_GOOGLE_BENCHMARK_GIT_REPOSITORY=https://github.com/google/benchmark.git \
  -DLUABRIDGE_GOOGLE_BENCHMARK_GIT_TAG=v1.8.4
```

## Run Benchmarks

Each executable supports standard Google Benchmark CLI flags.

```bash
./Build/Benchmarks/LuaBridge3Benchmark --benchmark_out=Build/Benchmarks/luabridge3.json --benchmark_out_format=json
./Build/Benchmarks/LuaBridgeVanillaBenchmark --benchmark_out=Build/Benchmarks/luabridge_vanilla.json --benchmark_out_format=json
./Build/Benchmarks/Sol3Benchmark --benchmark_out=Build/Benchmarks/sol3.json --benchmark_out_format=json  # if enabled
```

Recommended consistency flags for fair comparison:

```bash
--benchmark_min_time=0.1 --benchmark_repetitions=5
```

## Plot Results

The script `plot_benchmarks.py` merges one or more Google Benchmark JSON files and generates a grouped comparison chart.

```bash
python3 Benchmarks/plot_benchmarks.py \
  --input Build/Benchmarks/luabridge3.json Build/Benchmarks/luabridge_vanilla.json Build/Benchmarks/sol3.json \
  --output Build/Benchmarks/lua_bindings_comparison.png
```

Outputs:

- PNG chart (grouped bars, lower is better)
- Optional skipped/error report file next to the image (`*_skipped.txt`)

## Notes

- Some vanilla LuaBridge benchmarks are marked as skipped where the feature is unsupported.
- Sol3 target is optional (`LUABRIDGE_BENCHMARK_WITH_SOL3`) because current sol2 headers can fail to compile on some toolchains.
- If you need stricter reproducibility, pin all FetchContent dependencies to commits instead of branches.
