# Kryvo

Kryvo is an open-source file encryptor.

[![Build Status](https://travis-ci.org/adolby/Kryvo.svg?branch=master)](https://travis-ci.org/adolby/Kryvo) [![Build status](https://ci.appveyor.com/api/projects/status/tefc4ijnl7el4tko/branch/master?svg=true)](https://ci.appveyor.com/project/adolby/kryvo/branch/master)

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

Kryvo uses encryption provider plugins to interface with encryption libraries.

The Botan encryption provider plugin includes and builds the source files of Botan 2.7 (amalgamation) and zlib 1.2.11.

Amalgamation configurations of Botan are included in the source tree to simplify the build setup.

Android requires a small patch that implements std::string to_string(), which is missing from the NDK STL version 10e. The patch, android_to_string.hpp, is included by botan_all.h. Both files are located at src/plugins/cryptography/botan/android/.

If you would like to update Botan, you'll need to produce a new amalgamation build. The configuration commands used to generate Botan amalgamation files follows for each platform:

### Android ARM 32-bit
python configure.py --cpu=armv5te --os=linux --amalgamation --disable-shared --disable-modules=pkcs11 --with-zlib

### Linux GCC x86_64
python configure.py --cc=gcc --amalgamation --disable-shared --disable-modules=pkcs11 --with-zlib

### Linux Clang x86_64
python configure.py --cc=clang --amalgamation --disable-shared --disable-modules=pkcs11 --with-zlib

### macOS Clang x86_64
python configure.py --cc=clang --amalgamation --disable-shared --disable-modules=pkcs11 --with-zlib

### iOS Clang 32-bit
python configure.py --os=ios --cpu=armv8-a --cc=clang --amalgamation --disable-shared --disable-modules=pkcs11 --with-zlib

### Windows Visual Studio x86_64
python configure.py --cpu=x64 --cc=msvc --os=windows --amalgamation --disable-shared --disable-modules=pkcs11 --with-zlib

## Contact

You can contact the project creator via email at andrewdolby@gmail.com.
