#Kryvos
[![Build Status](https://travis-ci.org/adolby/Kryvos.svg?branch=master)](https://travis-ci.org/adolby/Kryvos) [![Build status](https://ci.appveyor.com/api/projects/status/tefc4ijnl7el4tko/branch/master?svg=true)](https://ci.appveyor.com/project/adolby/kryvos/branch/master)

Kryvos is an open-source file encryptor and decryptor. It encrypts files with AES and Serpent using the Botan cryptography library.

##Getting Started

To install Krvyos, run the installer executable or extract the files from the Kryvos archive. Once the install files are copied, run the Kryvos executable.

Add files by clicking/tapping the Add Files button and selecting them in the dialog or by dragging and dropping files to the Kryvos window. Click encrypt or decrypt to process the entire list.

Files are encrypted/decrypted one-to-one, meaning that encryption of a "file.txt" will yield an encrypted "file.txt.enc" and decryption of "file.txt.enc" will yield a decrypted "file (2).txt".

##Theming

Kryvos supports user theming. To create your own theme, start with the kryvos.qss file in the themes directory as your template. The kryvos.qss file is a stylesheet file that has syntax very similar to CSS.

Colors, fonts, and many other attributes can be styled. Save your stylesheet file inside of the themes folder. To apply your theme to Kryvos, edit the settings.ini file located in the same directory as kryvos.exe. Change the value styleSheetPath to your newly created stylesheet file.

Example: You created a new theme and saved the stylesheet to myTheme.qss in the themes folder. You would then update settings.json with this value: styleSheetPath: myTheme.qss

##Licenses

Kryvos is licensed under the MIT License. Read the LICENSE file or go to https://opensource.org/licenses/MIT for more information about the license.

Botan is licensed under the Simplified BSD License. Read the Botan License file for more information about the license. The license file can also be found at https://botan.randombit.net/license.txt.
Botan source code is available at https://botan.randombit.net/download.html.

Qt is licensed under the GNU Lesser General Public License version 2.1. Read the Qt License file or go to https://www.gnu.org/licenses/lgpl-2.1.html for more information about the license.
Qt source code is available at https://code.qt.io.

##Developers

If you'd like to contribute to Kryvos, you can fork the project on GitHub and submit a pull request. To build Kryvos, you'll need Qt 5.2 (or later), Botan 1.11.24, and a C++14 capable compiler.

Amalgamation configurations of Botan are included in the source tree to simplify the build setup. Android requires a small patch that implements std::string to_string(), which is missing from the NDK STL. The patch, android_to_string.hpp, is included by botan_all.h. Both files are located at src/cryptography/botan/android/. The configuration commands used to generate minimal Botan amalgamation files follows for each platform:

###Android (ARM)
python configure.py --cpu=armv5te --os=linux --via-amalgamation --minimized-build --disable-shared --enable-modules=aes,base,base64,dev_random,eax,filters,gcm,kdf2,keccak,locking_allocator,pbkdf2,serpent,serpent_simd

###Linux GCC x86_64
python configure.py --cc=gcc --via-amalgamation --minimized-build --disable-shared --enable-modules=aes,base,base64,dev_random,eax,filters,gcm,kdf2,keccak,locking_allocator,pbkdf2,serpent,serpent_simd

###Linux Clang x86_64
python configure.py --cc=clang --via-amalgamation --minimized-build --disable-shared --enable-modules=aes,base,base64,dev_random,eax,filters,gcm,kdf2,keccak,locking_allocator,pbkdf2,serpent,serpent_simd

###Mac OS X Clang x86_64
python configure.py --cc=clang --via-amalgamation --minimized-build --disable-shared --enable-modules=aes,base,base64,dev_random,eax,filters,gcm,kdf2,keccak,locking_allocator,pbkdf2,serpent,serpent_simd

###iOS Clang
python configure.py --cpu=armv7 --cc=clang --via-amalgamation --minimized-build --disable-shared --enable-modules=aes,base,base64,dev_random,eax,filters,gcm,kdf2,keccak,locking_allocator,pbkdf2,serpent,serpent_simd

###Windows Visual Studio x86_64
python configure.py --cpu=x64 --cc=msvc --via-amalgamation --minimized-build --disable-shared --enable-modules=aes,base,base64,eax,filters,gcm,kdf2,keccak,pbkdf2,serpent,serpent_simd,win32_stats

###Windows Visual Studio x86
python configure.py --cpu=x86 --cc=msvc --via-amalgamation --minimized-build --disable-shared --enable-modules=aes,base,base64,eax,filters,gcm,kdf2,keccak,pbkdf2,serpent,serpent_simd,win32_stats

###Windows MinGW-w64 GCC x86_64
python configure.py --cpu=x64 --cc=gcc --os=mingw --via-amalgamation --minimized-build --disable-shared --enable-modules=aes,base,base64,eax,filters,gcm,kdf2,keccak,pbkdf2,serpent,serpent_simd,win32_stats

###Windows MinGW GCC x86
python configure.py --cpu=x86 --cc=gcc --os=mingw --via-amalgamation --minimized-build --disable-shared --enable-modules=aes,base,base64,eax,filters,gcm,kdf2,keccak,pbkdf2,serpent,win32_stats

##Contact

You can contact the project creator via email at andrewdolby@gmail.com.
