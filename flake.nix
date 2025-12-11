{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
    in
    flake-utils.lib.eachSystem supportedSystems (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        # Final derivation including any overrides made to output package
        inherit (self.packages.${system}) kcli kcli-gcc;

        devPkgs = with pkgs; [
          shfmt
          cmake-format
          clang-tools # NOTE: clang-tools must come before clang
          clang
        ];

        mkApp = text: {
          type = "app";
          program = pkgs.lib.getExe (
            pkgs.writeShellApplication {
              name = "app";
              runtimeInputs = devPkgs;
              inherit text;
            }
          );
        };
      in
      {
        packages = {
          kcli = pkgs.callPackage ./. {
            stdenv = pkgs.clangStdenv;
          };

          kcli-gcc = kcli.override {
            inherit (pkgs) stdenv;
          };

          kcli-win = kcli.override {
            inherit (pkgs.pkgsCross.mingwW64) stdenv;
          };

          default = kcli;

          kcli-test = kcli.override {
            doCheck = true;
          };
          kcli-gcc-test = kcli-gcc.override {
            doCheck = true;
          };
        };

        devShells = {
          default = pkgs.mkShell {
            inputsFrom = [ kcli ];
            nativeBuildInputs = devPkgs;
            shellHook = ''
              source dev_shell.sh
            '';
          };
        };

        apps = {
          format = mkApp ''
            ./format.sh
          '';
        };

        formatter = pkgs.nixfmt-rfc-style;
      }
    );
}
