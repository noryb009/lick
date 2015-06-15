# Building LICK

LICK can be built on either Linux or Windows.

Dependencies:
- MinGW (gcc, make)
- CMake
- NSIS
- FLTK library

1. Obtain the source code from https://github.com/noryb009/lick
2. Populate the res directory. There are two ways to do this:
    1. Download a release zip of LICK, and copy the res directory.
    2. Build the resources. See the building resources section.

Build with `cmake -DCMAKE_BUILD_TYPE=Release .`, then `make`. To create the installer and zip package, run
`make package`. Note on Windows, `make` may be named `mingw32-make`.

# Building Resources
## grub4dos
LICK uses a patched version of grub4dos, where
- grub.exe is renamed to pupl.exe
- grldr is renamed to pupldr
- grldr.mbr is renamed to pupldr.mbr

You can download the patched version from https://github.com/noryb009/grub4dos
in the pupldr branch.

Build grub4dos by running:

```
./autogen.sh
make
```

Copy stage2/pupl.exe, stage2/pupldr, and stage2/pupldr.mbr to res/.

You may want to include a GUI, which should be named grub4dos-gui.gz and
placed in res/.

## grub2
Download the latest version of grub2 for Windows from
ftp://ftp.gnu.org/gnu/grub/ or http://alpha.gnu.org/gnu/grub/
At the time of writing, the former link does not have builds for Windows.

Copy locale/, themes/, x86_64-efi/, COPYING, grub-install.exe, and unicode.pf2
to res/grub2/. To fulfill the requirements of the GPL3 license, you may want
to include a file to point users to the source code of grub2.

## PreLoader
Including PreLoader is optional, it allows grub2 to boot without having to
disable secure boot. Download PreLoader.efi and HashTool.efi from
http://blog.hansenpartnership.com/linux-foundation-secure-boot-system-released/
and copy them to res/.
