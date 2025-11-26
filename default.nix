{
  cmake,
  lib,
  stdenv,
  doCheck ? false,
}:
stdenv.mkDerivation {
  name = "kcli";
  src = lib.cleanSource ./.;
  inherit doCheck;

  nativeBuildInputs = [ cmake ];

  configurePhase = ''
    cmake -B build
  '';

  buildPhase = ''
    cmake --build build
  '';

  installPhase = ''
    if [[ "$CC" == *"mingw32"* ]]; then
      # Workaround broken pkgCross cmake install
      mkdir -p "$out/bin"
      cp build/app/*.exe "$out/bin"
      mkdir -p "$out/lib"
      cp build/src/*.a "$out/lib"
    else
      cmake --install build --prefix $out
    fi

    mkdir -p "$out/include"
    cp include/*.h "$out/include"
  '';

  checkPhase = ''
    (
      cd build/test
      ctest --output-on-failure
    )
  '';
}
