{
  description = "Atlas engine";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachSystem [ "aarch64-darwin" "aarch64-linux" "x86_64-darwin" "x86_64-linux" ] (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        stdenv = pkgs.llvmPackages.stdenv;
      in
      {
        packages.default = stdenv.mkDerivation {
          pname = "atlas";
          version = "0.1.0";
          src = ./.;

          buildInputs = with pkgs; [
            taskflow
          ];

          nativeBuildInputs = with pkgs; [
            clang-tools
            ninja
            cmake
            gtest
          ];

          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
            "-DBUILD_TESTS=ON"
            "-DENABLE_INSTALL=ON"
          ];

          doCheck = true;
          checkPhase = ''
            ctest --output-on-failure
          '';
        };

        # Expose the entire source for the package so that it can be added as a cmake subdirectory in game projects using atlas and nix flakes to build.
        src = ./.;
      }
    );
}
