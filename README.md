# Kryvo

Kryvo is a cross-platform open source file encryptor.

## Getting Started

### Download
[Download Kryvo](https://github.com/adolby/Kryvo/releases) for macOS, Windows, or Linux.

### Installation
To install Kryvo on Windows or Linux, run the installer executable or extract the files from the Kryvo portable archive. To install Kryvo on macOS, extract the disk image archive, then mount the disk image. Drag Kryvo.app to your Applications folder. Once the install files are copied, run the Kryvo executable.

### Solving Windows missing DLL error
If you receive an error indicating a missing msvcp140.dll, vcruntime140.dll, or other DLL files when you run Kryvo, you'll need to install the Visual C++ 2015 Redistributable (x64).

The Visual C++ Redistributable installer is located where you installed Kryvo (default directory for the installer is C:\Program Files\Kryvo\). Its file name is vcredist_x64.exe.

Run the Visual C++ Redistributable installer, and you should then be able to start Kryvo.

### Using Kryvo
Add files by clicking/tapping the Add Files button and selecting them in the dialog or by dragging and dropping files to the Kryvo window. Press encrypt or decrypt to process the entire list.

Files are encrypted/decrypted one-to-one, meaning that encryption of a "file.txt" will yield an encrypted "file.txt.enc" and decryption of "file.txt.enc" will yield a decrypted "file (2).txt".

## Theming

Kryvo supports user theming on desktop platforms. To create your own theme, start with the kryvo.qss file in the themes directory as your template. The kryvo.qss file is a stylesheet file that has syntax very similar to CSS.

Colors, fonts, and many other attributes can be styled. Save your stylesheet file inside of the themes folder. To apply your theme to Kryvo, edit the settings.ini file located in the same directory as the Kryvo executable. Change the value styleSheetPath to your newly created stylesheet file.

Example: You created a new theme and saved the stylesheet to myTheme.qss in the themes folder. You would then update settings.json with this value: styleSheetPath: myTheme.qss

## Plugins

Kryvo uses cryptography provider plugins to perform cryptographic operations. The default cryptography provider is [OpenSSL](https://www.openssl.org/).

## Licenses

Kryvo is licensed under the MIT License. Read the LICENSE file or go to https://opensource.org/licenses/MIT for more information about the license.

OpenSSL is licensed under the Apache License Version 2.0. Read the OpenSSL LICENSE.txt file for more information about the license. OpenSSL source code is available at: https://www.openssl.org

Botan is licensed under the Simplified BSD License. Read the Botan License file for more information about the license. The license file can also be found at https://botan.randombit.net/license.txt.
Botan source code is available at https://botan.randombit.net/download.html.

Qt is licensed under the GNU Lesser General Public License version 2.1. Read the Qt License file or go to https://www.gnu.org/licenses/lgpl-2.1.html for more information about the license.
Qt source code is available at https://code.qt.io.

## Developers / Contributing

If you'd like to contribute to Kryvo, you can fork the project on GitHub and submit a pull request.

### Building from source
To build Kryvo, you'll need Qt 5.2 (or later), and a C++14 capable compiler.

Development is performed in Qt Creator with qmake as the Makefile generator.

### Plugins
Kryvo uses encryption provider plugins to interface with encryption libraries.

Plugins can be built as static or dynamic libraries that are loaded at runtime. The OpenSSL and Botan provider plugins are compiled as static libraries to be always available. A build script will be required to copy dynamic plugins into the app build directory. The build_macOS.sh, build_linux.sh, and build_windows.cmd scripts will copy plugins and produce a working app build as output.

### OpenSSL
The OpenSSL encryption provider plugin uses header and library files from the Qt OpenSSL 3 installer package on Windows and Linux. On macOS it uses the OpenSSL 3 brew package.

### Botan
The Botan encryption provider plugin includes and builds the source files of Botan 2.9 (amalgamation).

## Contact

You can contact the project creator via email at andrewdolby@gmail.com.
