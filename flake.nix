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
      systems = [ "x86_64-linux" ];
      forEachSystem = f: (nixpkgs.lib.genAttrs systems (system: (f system)));
    in {
      defaultPackage = (
        forEachSystem (system: (
          let nix_pkgs = (import nixpkgs { system = "${system}"; }); in (
            nix_pkgs.stdenv.mkDerivation {
              name = "lick";
              src = self;
              buildInputs = [ nix_pkgs.pkgs.cmake nix_pkgs.pkgs.xorg.libX11 nix_pkgs.pkgs.fltk ];
              configurePhase = ''
                cmake .
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
              installPhase = ''
                make package
                mkdir -p $out
                install -t $out LICK-*.zip;
                # Linux does not create `.exe` files.
                to_install=(LICK-*.exe)
                for f in "''${to_install[@]}"; do install -t $out "$f"; done
              '';
            }
          )
        ))
      );
    }
  );
}
