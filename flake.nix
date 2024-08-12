{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    systems.url = "github:nix-systems/default";
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = { nixpkgs, systems, treefmt-nix, ... }:
    let
      eachSystem = f: nixpkgs.lib.genAttrs (import systems) (system: f
        (import nixpkgs {
          inherit system;
          config = { };
        })
      );
      treefmtEval = eachSystem (pkgs: treefmt-nix.lib.evalModule pkgs (_: {
        projectRootFile = "flake.nix";
        programs = {
          clang-format.enable = true;
          deadnix.enable = true;
          nixpkgs-fmt.enable = true;
          statix.enable = true;
        };
        settings.global.excludes = [ "vendor/*" ];
      }));
    in
    {
      devShells = eachSystem (pkgs: {
        default = with pkgs; mkShell {
          packages = [
            bear
            clang-tools
            lldb
            treefmtEval.${system}.config.build.wrapper
          ] ++ lib.optionals stdenv.isLinux [
            gdb
            linuxPackages.perf
            vulkan-tools
            vulkan-tools-lunarg
          ];
          # There appears to be an issue when using wayland for me. Without
          # this, the window doesn't show up.
          shellHook = ''
            unset WAYLAND_DISPLAY
          '';
          nativeBuildInputs = [
            pkg-config
          ];
          buildInputs = [
            raylib
          ];
          LD_LIBRARY_PATH = lib.makeLibraryPath [
            libxkbcommon
            vulkan-loader
            wayland
          ];
        };
      });
    };
}
