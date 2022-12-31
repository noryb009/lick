{
  description = "LICK: Install Puppy Linux in a few clicks";

  inputs = {
    nixpkgs.follows = "nix/nixpkgs";
    fltk_tarball = {
      type = "file";
      url = "https://www.fltk.org/pub/fltk/1.3.5/fltk-1.3.5-source.tar.bz2";
      flake = false;
    };
    libarchive_tarball = {
      type = "file";
      url = "https://libarchive.org/downloads/libarchive-3.3.1.tar.gz";
      flake = false;
    };
    catch_header = {
      type = "file";
      url = "https://github.com/catchorg/Catch2/releases/download/v2.13.9/catch.hpp";
      flake = false;
    };
  };

  outputs = { self, nix, nixpkgs, fltk_tarball, libarchive_tarball, catch_header }: (
    let
      systems = [ "x86_64-linux" "mingw32" ];
      forEachItem = items: f: (nixpkgs.lib.genAttrs items (item: (f item)));
      forEachSystem = forEachItem systems;
      project = nix_pkgs: {
        name = "lick";
        src = self;
        buildInputs = ([]
          ++ nix_pkgs.lib.optional nix_pkgs.targetPlatform.isLinux [
            nix_pkgs.pkgs.xorg.libX11
          ]
        );
        nativeBuildInputs = [
          nix_pkgs.pkgs.buildPackages.cmake
          nix_pkgs.pkgs.buildPackages.fltk
          nix_pkgs.pkgs.buildPackages.nsis
        ];
        preConfigurePhase = ''
          # Write nix cmake flags (including cross-compilation flags) to file.
          touch nix-cmake-config.cmake
          for i in $cmakeFlags; do
            i="''${i:2}"
            echo "set(''${i/%=*/} ''${i/#*=/})" >> nix-cmake-config.cmake
          done
        '';
        configurePhase = ''
          cmake . -DCMAKE_TOOLCHAIN_FILE=nix-cmake-config.cmake -DCMAKE_BUILD_TYPE=Release
        '';
        preBuildPhase = ''
          # Copy external dependencies to avoid downloading them.
          mkdir -p libarchive-3.3.1-prefix/src/
          cp ${libarchive_tarball} libarchive-3.3.1-prefix/src/libarchive-3.3.1.tar.gz
          mkdir -p fltk-1.3.5-prefix/src/
          cp ${fltk_tarball} fltk-1.3.5-prefix/src/fltk-1.3.5-source.tar.bz2
          mkdir -p test/catch-2.13.9/src/
          cp ${catch_header} test/catch-2.13.9/src/catch.hpp
        '';
        buildPhase = ''
          make
        '';
        checkPhase = ''
          make test
        '';
        installPhase = ''
          make package
          mkdir -p $out
          install -t $out LICK-*.zip;
          # Linux does not create `.exe` files.
          to_install=(LICK-*.exe)
          for f in "''${to_install[@]}"; do install -t $out "$f"; done
        '';
      };
    in {
      packages = (
        forEachSystem (system: (
          let
            nix_pkgs = (import nixpkgs { system = "${system}"; });
            otherSystems = (nixpkgs.lib.filter (x: x != system) systems);
            forEachOtherSystem = forEachItem otherSystems;
          in {
            default = nix_pkgs.stdenv.mkDerivation ((project nix_pkgs));
            pkgsCross = forEachOtherSystem (target: (
              let crossPkgs = nix_pkgs.pkgsCross."${target}";
              in crossPkgs.stdenv.mkDerivation (project crossPkgs)
            ));
          }
        ))
      );
    }
  );
}
