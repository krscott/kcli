# c-start

My rather opinionated C project template.

To start a project with this template, run:
```
./init-template.sh new_project_name
```

Don't forget to change the [LICENSE](https://choosealicense.com/).

## Development

Update dependencies
```
nix flake update
```

Requires CMake and a C11 compiler. A nix dev shell is available:
```
nix develop
```

Standard build
```
cmake -B build
cmake --build build
```

Useful development shell aliases
```
source dev_shell.sh

# Reconfigure cmake
cfg

# Build and run
run

# Setup vscode debugging
setup_vscode
```

### Debugging

Example [`launch.json`](dev/vscode/launch.json) and 
[`tasks.json`](dev/vscode/tasks.json) files are included to debug in vscode.

Nix home-manager setup
```nix
    programs.vscode = {
      enable = true;
      package = pkgs.vscodium;
      profiles.default.extensions = with pkgs.vscode-extensions; [
        llvm-vs-code-extensions.lldb-dap
        llvm-vs-code-extensions.vscode-clangd
      ];
    };
```
