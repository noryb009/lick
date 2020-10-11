Traditionally, this was compiled using mingw-i686 in Arch Linux. I have lost the commands on
how I set this up.

In Ubuntu (WSL), you can run this:

Setup:
```
# Install mingw.
sudo apt install g++-mingw-w64
# Install fluid.
sudo apt install fluid
# Install nsis. I use 2.51, 3.x might not work with Windows 9x. This is optional if you only want the `.zip` package.
suto apt install nsis
```

Build:
```
# Create build directory
mkdir build
cd build
# Configure
PATH="/usr/i686-w64-mingw32/:$PATH" cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREIFX:PATH=/usr/local/i686-w64-mingw32 -DCMAKE_INSTALL_LIBDIR:PATH=lib -DCMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES:PATH=/usr/i686-w64-mingw32/include -DCMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES:PATH=/usr/i686-w64-mingw32/include -DCMAKE_TOOLCHAIN_FILE=../mingw-config.cmake ..
# Make
make
# Build packages
make package
```