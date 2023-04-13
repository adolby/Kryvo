# Botan provider plugin

The Botan encryption provider plugin includes and builds the source files of Botan 2.9 (amalgamation).

Amalgamation configurations of Botan are included in the source tree to simplify the build setup.

## Updating Botan
If you would like to update Botan, you'll need to produce a new amalgamation build. The configuration commands used to generate Botan amalgamation files follows for each platform:

### Android ARM armv7
python configure.py --cpu=arm32 --os=android --amalgamation --disable-shared --disable-modules=pkcs11 --disable-neon

### Linux GCC x86_64
python configure.py --cc=gcc --amalgamation --disable-shared --disable-modules=pkcs11

### Linux Clang x86_64
python configure.py --cc=clang --amalgamation --disable-shared --disable-modules=pkcs11

### macOS Clang x86_64
python configure.py --cc=clang --amalgamation --disable-shared --disable-modules=pkcs11

### iOS Clang armv8-a
python configure.py --os=ios --cpu=arm64 --cc=clang --amalgamation --disable-shared --disable-modules=pkcs11

### Windows Visual Studio x86_64
python configure.py --cpu=x86_64 --cc=msvc --os=windows --amalgamation --disable-shared --disable-modules=pkcs11

### Windows Visual Studio x86
python configure.py --cpu=x86_32 --cc=msvc --os=windows --amalgamation --disable-shared --disable-modules=pkcs11

### Windows MinGW-w64 x86
python configure.py --cpu=x86_32 --cc=gcc --os=mingw --amalgamation --disable-shared --disable-modules=pkcs11
