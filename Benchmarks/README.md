# LuaBridge Benchmarks

This directory contains a standalone benchmark executable focused on LuaBridge call and property access overhead. The selected cases mirror the style of the sol2 benchmark categories (Lua -> C++ calls, C++ -> Lua calls, member calls, and property access).

## Build

From project root:

```bash
cmake -S . -B Build -DCMAKE_BUILD_TYPE=Release -DLUABRIDGE_BENCHMARKS=ON
cmake --build Build --target LuaBridgeBenchmarks --config Release
```

## Run

```bash
./Build/Benchmarks/LuaBridgeBenchmarks
# optional iterations
./Build/Benchmarks/LuaBridgeBenchmarks 5000000
```

The executable prints:
- `ns/op` (lower is better)
- `Mop/s` (higher is better)

## Optional sol2 Side-by-Side

To compare with sol2 in the same executable, enable the option below. The build will download sol2 through CMake FetchContent:

```bash
cmake -S . -B Build \
  -DCMAKE_BUILD_TYPE=Release \
  -DLUABRIDGE_BENCHMARKS=ON \
  -DLUABRIDGE_BENCHMARK_WITH_SOL2=ON
cmake --build Build --target LuaBridgeBenchmarks --config Release
```

By default, FetchContent uses:
- Repository: `https://github.com/ThePhD/sol2.git`
- Tag: `v3.3.0`

You can override these if needed:

```bash
cmake -S . -B Build \
  -DCMAKE_BUILD_TYPE=Release \
  -DLUABRIDGE_BENCHMARKS=ON \
  -DLUABRIDGE_BENCHMARK_WITH_SOL2=ON \
  -DLUABRIDGE_SOL2_GIT_REPOSITORY=https://github.com/ThePhD/sol2.git \
  -DLUABRIDGE_SOL2_GIT_TAG=v3.3.0
```

When enabled, the benchmark prints LuaBridge and sol2 rows for the same cases and iteration count.

## Notes for Fair Comparison

- Use Release mode and the same compiler for all libraries.
- Pin CPU frequency/governor where possible and avoid background load.
- Run each benchmark multiple times and compare medians.
- Keep Lua version identical between LuaBridge and sol2 runs.
