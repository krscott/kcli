{
  cmake,
  lib,
  stdenv,
  doCheck ? false,
}:
stdenv.mkDerivation {
  name = "c-start";
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
    else
      cmake --install build --prefix $out
    fi
  '';

  checkPhase = ''
    (
      cd build/test
      ctest --output-on-failure
    )
  '';
}
