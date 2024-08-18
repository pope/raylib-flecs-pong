{
  description = "Logic Farm Roguelike project";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default";
    rust-overlay = {
      url = "github:oxalica/rust-overlay";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = { nixpkgs, systems, rust-overlay, treefmt-nix, ... }:
    let
      overlays = [ (import rust-overlay) ];
      eachSystem = f: nixpkgs.lib.genAttrs (import systems) (system: f
        (import nixpkgs {
          inherit system overlays;
          config = { };
        })
      );
      treefmtEval = eachSystem (pkgs: treefmt-nix.lib.evalModule pkgs (_: {
        projectRootFile = "flake.nix";
        programs = {
          deadnix.enable = true;
          nixpkgs-fmt.enable = true;
          statix.enable = true;
        };
      }));
    in
    {
      devShells = eachSystem (pkgs:
        {
          default = with pkgs; mkShell rec {
            nativeBuildInputs = [
              clang
              cmake
              pkg-config
              rust-analyzer
              rust-bin.stable.latest.default
              rustfmt
              treefmtEval.${system}.config.build.wrapper
            ];
            buildInputs = [glfw] ++ lib.optionals stdenv.isLinux [
              alsa-lib
              libGL
              libGLU
              libxkbcommon
              udev
              vulkan-loader
              xorg.libX11
              xorg.libXcursor
              xorg.libXi
              xorg.libXinerama
              xorg.libXrandr
              wayland
            ];
            LD_LIBRARY_PATH = lib.makeLibraryPath buildInputs;
            LIBCLANG_PATH = "${pkgs.libclang.lib}/lib";
          };
        }
      );

      formatter = eachSystem (pkgs: treefmtEval.${pkgs.system}.config.build.wrapper);

      checks = eachSystem (pkgs: {
        formatting = treefmtEval.${pkgs.system}.config.build.wrapper;
      });
    };
}
