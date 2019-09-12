# Kryvo

Kryvo is an open-source file encryptor.

[![Build Status](https://travis-ci.com/adolby/Kryvo.svg?branch=master)](https://travis-ci.com/adolby/Kryvo) [![Build status](https://ci.appveyor.com/api/projects/status/9idpva2lb6w4pwpe/branch/master?svg=true)](https://ci.appveyor.com/project/adolby/kryvo/branch/master)

## Getting Started

### Download
[Download Kryvo](https://github.com/adolby/Kryvo/releases) for macOS, Windows, or Linux.

### Installation
To install Kryvo on Windows or Linux, run the installer executable or extract the files from the Kryvo archive. To install Kryvo on macOS, extract the disk image archive, then mount the disk image. Drag Kryvo.app to your Applications folder. Once the install files are copied, run the Kryvo executable.

### Solving Windows missing DLL error
If you receive an error indicating a missing msvcp140.dll, vcruntime140.dll, or other DLL files when you run Kryvo, you'll need to install the Visual C++ 2015 Redistributable (x64).

The Visual C++ Redistributable installer is located where you installed Kryvo (default directory for the installer is C:\Program Files\Kryvo\). Its file name is vcredist_x64.exe.

Run the Visual C++ Redistributable installer, and you should then be able to start Kryvo.

### Using Kryvo
Add files by clicking/tapping the Add Files button and selecting them in the dialog or by dragging and dropping files to the Kryvo window. Press encrypt or decrypt to process the entire list.

Files are encrypted/decrypted one-to-one, meaning that encryption of a "file.txt" will yield an encrypted "file.txt.enc" and decryption of "file.txt.enc" will yield a decrypted "file (2).txt".

## Theming

Kryvo supports user theming. To create your own theme, start with the kryvo.qss file in the themes directory as your template. The kryvo.qss file is a stylesheet file that has syntax very similar to CSS.

Colors, fonts, and many other attributes can be styled. Save your stylesheet file inside of the themes folder. To apply your theme to Kryvo, edit the settings.ini file located in the same directory as kryvo.exe. Change the value styleSheetPath to your newly created stylesheet file.

Example: You created a new theme and saved the stylesheet to myTheme.qss in the themes folder. You would then update settings.json with this value: styleSheetPath: myTheme.qss

## Plugins

Kryvo uses cryptography provider plugins to perform cryptographic operations.

The default cryptography provider is [Botan](https://botan.randombit.net/).

## Licenses

Kryvo is licensed under the MIT License. Read the LICENSE file or go to https://opensource.org/licenses/MIT for more information about the license.

Botan is licensed under the Simplified BSD License. Read the Botan License file for more information about the license. The license file can also be found at https://botan.randombit.net/license.txt.
Botan source code is available at https://botan.randombit.net/download.html.

Qt is licensed under the GNU Lesser General Public License version 2.1. Read the Qt License file or go to https://www.gnu.org/licenses/lgpl-2.1.html for more information about the license.
Qt source code is available at https://code.qt.io.

## Developers / Contributing

If you'd like to contribute to Kryvo, you can fork the project on GitHub and submit a pull request.

## Building from source
To build Kryvo, you'll need Qt 5.2 (or later), and a C++14 capable compiler.

Development is performed in Qt Creator with qmake as the Makefile generator.

### Plugins
Kryvo uses encryption provider plugins to interface with encryption libraries.

Currently there is only the Botan plugin which is built staticly with the application to ensure it is always available. Future plugins can be built as static or dynamic libraries that are loaded at runtime. A build script will be required to copy dynamic plugins into the app build directory. The build_macOS.sh, build_linux.sh, and build_windows.cmd scripts will copy plugins and produce a working app build as output.

### Botan
The Botan encryption provider plugin includes and builds the source files of Botan 2.9 (amalgamation).

Amalgamation configurations of Botan are included in the source tree to simplify the build setup.

### Updating Botan
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

## Contact

You can contact the project creator via email at andrewdolby@gmail.com.
