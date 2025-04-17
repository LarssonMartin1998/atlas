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
        pkgsCross = pkgs.pkgsCross.mingwW64;

        stdenv = pkgs.llvmPackages.stdenv;
        xstdenv = pkgsCross.stdenv;

        pname = "atlas";
        version = "0.1.0";
      in
      {
        packages.atlas = stdenv.mkDerivation {
          pname = pname;
          version = version;
          src = ./.;

          buildInputs =
            with pkgs;
            [
              vulkan-headers
              vulkan-loader
            ]
            ++ lib.optional stdenv.isDarwin [ moltenvk ];

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

          # Needed on Linux: libvulkan is loaded with dlopen()
          # so it is *not* seen by patchelf → keep the RPATH.
          dontPatchELF = stdenv.isLinux; # :contentReference[oaicite:0]{index=0}

          doCheck = true;
          checkPhase = ''
            ctest --output-on-failure
          '';
        };

        packages.atlas-windows = xstdenv.mkDerivation {
          pname = pname;
          version = version;
          src = ./.;

          buildInputs = with pkgsCross; [
            vulkan-headers
            vulkan-loader
            glfw
          ];

          nativeBuildInputs = with pkgs; [
            clang-tools
            ninja
            cmake
          ];

          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
            "-DBUILD_TESTS=OFF"
            "-DENABLE_INSTALL=ON"
          ];
        };

        # Expose the entire source for the package so that it can be added as a cmake subdirectory in game projects using atlas and nix flakes to build.
        src = ./.;

        packages.default = self.packages.${system}.atlas;
      }
    );
}
