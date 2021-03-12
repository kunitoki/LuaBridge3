# LuaBridge Unit Tests

## Linux and MacOS

Have CMake 3.5+ and a compiler supporting C++17 installed.

Run cmake to generate Makefiles:

```sh
mkdir build
cd build
cmake -G "Unix Makefiles" ../
```

Build the project:

```sh
cmake --build . -- -j8
```

Run tests from the `build` directory:

```sh
./Tests/LuaBridgeTests51
./Tests/LuaBridgeTests51Noexcept
./Tests/LuaBridgeTests52
./Tests/LuaBridgeTests52Noexcept
./Tests/LuaBridgeTests53
./Tests/LuaBridgeTests53Noexcept
./Tests/LuaBridgeTests54
./Tests/LuaBridgeTests54Noexcept
```

## macOS

Have CMake 3.5+ and Xcode 11+ installed.

Run cmake to generate Makefiles:

```sh
mkdir build
cd build
cmake -G "Xcode" ../
```

Build the project:

```sh
cmake --build . -- -j8
```

Run tests from the `build` directory:

```sh
./Tests/LuaBridgeTests51
./Tests/LuaBridgeTests51Noexcept
./Tests/LuaBridgeTests52
./Tests/LuaBridgeTests52Noexcept
./Tests/LuaBridgeTests53
./Tests/LuaBridgeTests53Noexcept
./Tests/LuaBridgeTests54
./Tests/LuaBridgeTests54Noexcept
```

# Windows

Have CMake 3.5+ and MSVC 2019 installed.

Run cmake to generate MSVC solution and project files (run `cmake -G` to see all variants):

```cmd
mkdir build
cd build
cmake -G "Visual Studio 16" ../
```

Open the solution `LuaBridge.sln` in MSVC.

Set `LuaBridgeTests51` or `LuaBridgeTests52` or `LuaBridgeTests53` or `LuaBridgeTests54` or their corresponding Noexcept version
as a startup project and run.
