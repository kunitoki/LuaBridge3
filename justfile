default:
    @just -l

generate:
    cmake -G Xcode -B Build .

clean:
    rm -rf Build
