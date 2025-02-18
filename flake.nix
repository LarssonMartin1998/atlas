{
  description = "Atlas engine";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      packages.${system}.default = pkgs.stdenv.mkDerivation {
        pname = "atlas";
        version = "0.1.0";
        src = ./.;

        nativeBuildInputs = with pkgs; [
          clang
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
    };
}
