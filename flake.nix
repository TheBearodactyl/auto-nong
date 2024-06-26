{
  description = "GUI fetch tool written in Flutter for Linux.";

  inputs = {
    # nixpkgs.url = "github:NixOS/nixpkgs/bd645e8668ec6612439a9ee7e71f7eac4099d4f6";
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    flake-utils,
    nixpkgs,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        inherit system;
        # crossSystem = {
        #   # config = "x86_64-w64-mingw32";
        #   # config = "x86_64-unknown-windows-msvc";
        # };
      };
    in {
      packages = {
        geode-cli = pkgs.callPackage ./nix/geode-cli.nix {};
        xwin = pkgs.callPackage ./nix/xwin.nix {};
      };
      devShell =
        (pkgs.buildFHSUserEnv {
          name = "devshell";
          targetPkgs = pkgs: (with pkgs; [
            cmake
            # clang_17
            # clang_17
            llvmPackages_17.clang-unwrapped
            # clang-tools_17
            lld_17
            llvm_17
            pkg-config
            glibc
            # ninja
            # libgcc.lib
            libgcc
            self.packages.${system}.geode-cli
            self.packages.${system}.xwin
          ]);
          runScript = let
            jukeboxBindings = pkgs.fetchFromGitHub {
              owner = "Fleeym";
              repo = "sapphire-bindings";
              rev = "3d4040749c1b96956fc4a460b91abbacc53db75b";
              hash = "sha256-/UAugKml1iixIQ/JRVTO5Nic8lYV2P87/PregG+/4jQ=";
            };
          in pkgs.writeShellScript "fhs-script" ''
            export GEODE_SDK="$HOME/Documents/Geode/"
            export SPLAT_DIR=$HOME/.cache/geode/splat
            export TOOLCHAIN=$HOME/.cache/geode/toolchain/clang-cl-msvc.cmake
            # export GEODE_BINDINGS_REPO_PATH=${jukeboxBindings}

            # export PYTHON3DLL=/home/flafy/Downloads/python3-11.8-nuget/tools/python311.dll
            # export PYTHON3INCLUDES=/home/flafy/Downloads/python3-11.8-nuget/tools/include
            fish
          '';

          # env.;
        })
        .env;
      # devShell = pkgs.mkShell {
      #   nativeBuildInputs = with pkgs; [
      #     cmake
      #     # clang_17
      #     # clang_17
      #     llvmPackages_17.clang-unwrapped
      #     # clang-tools_17
      #     lld_17
      #     llvm_17
      #     pkg-config
      #     glibc
      #     libgcc.lib
      #     self.packages.${system}.geode-cli
      #     self.packages.${system}.xwin
      #   ];
      #   env = {
      #     # SKIP_BUILDING_CODEGEN = "OFF";
      #     GEODE_SDK = "/home/flafy/Documents/Geode/";
      #   };
      #   NIX_LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [
      #     pkgs.stdenv.cc.cc.lib
      #     pkgs.openssl
      #     pkgs.libgcc.lib
      #     # ...
      #   ];
      #   NIX_LD = pkgs.lib.fileContents "${pkgs.stdenv.cc}/nix-support/dynamic-linker";
      # };
    });
}
