cfg() {
    (
        set -eux

        rm -rf build/
        cmake -B build
    )
}

bld() {
    (
        set -eu

        if [ ! -d build ]; then
            cfg
        fi

        (
            set -x
            cmake --build build
        )

        if [ "$CMAKE_BUILD_TYPE" = "Debug" ]; then
            mkdir -p .compile-db
            cp build/compile_commands.json .compile-db
        fi
    )
}

run() {
    (
        set -eu

        bld

        set -x
        ./build/app/c-start "$@"
    )
}

crun() {
    (
        set -eu

        cfg
        run "$@"
    )
}

tst() {
    (
        set -eu

        bld
        cd build/test

        set -x
        ctest --output-on-failure
    )
}

ctst() {
    (
        set -eu

        cfg
        tst
    )
}

release() {
    unset DISABLE_OPTIMZATIONS
    unset CMAKE_BUILD_TYPE
}

debug() {
    release
    export CMAKE_BUILD_TYPE=Debug
}

o0() {
    debug
    export DISABLE_OPTIMIZATIONS=1
}

setup_vscode() {
    mkdir -p .vscode/
    cp dev/vscode/* .vscode/
}

fmt() {
    ./format.sh
}

export CC=clang
debug
