#Kryvos 0.9.8 [![Build Status](http://162.244.31.87:8080/buildStatus/icon?job=Kryvos)](http://162.244.31.87:8080/)

Kryvos is an open-source file encryptor and decryptor. It encrypts files with AES and Serpent using the Botan cryptography library.

##Getting Started

To install Krvyos, run the installer executable or extract the files from the Kryvos archive. Once the install files are copied, run the Kryvos executable. More detailed instructions can be found in the INSTALL file.

##Theming

Kryvos supports user theming. To create your own theme, start with the kryvos.qss file in the themes directory as your template. The kryvos.qss file is a stylesheet file that has syntax very similar to CSS. Colors, fonts, and many other attributes can be styled. Save your stylesheet file inside of the themes folder. To apply your theme to Kryvos, edit the settings.ini file located in the same directory as kryvos.exe. Change the value styleSheetPath to your newly created stylesheet file.

Example: You created a new theme and saved the stylesheet to myTheme.qss in the themes folder. You would then update settings.ini with this value: styleSheetPath=myTheme.qss

##Licenses

Kryvos is licensed under the GNU GPL License. Read LICENSE.GPL for more information about the license.
Kryvos source code and binaries are available on GitHub at https://github.com/adolby/Kryvos.

Botan is licensed under the BSD-2 License. Read the Botan License file for more information about the license. The license file can also be found at http://botan.randombit.net/license.html.
Botan source code is available at http://botan.randombit.net/download.html.

Qt is licensed under the GNU GPL License. Read LICENSE.GPL for more information about the license.
Qt source code is available at http://qt.gitorious.org/qt.

##Developers

If you'd like to contribute to Kryvos, check the project out on GitHub or submit a pull request. To build Kryvos, you'll need Qt 5.2 (or later) and Botan 1.11 (or later). Development is performed primarily in Qt Creator with gcc 4.9.2 on Linux and Windows (with MinGW for x86 and MinGW-w64 for x64 builds) and clang (packaged with Xcode 6) on Mac OS X.

Amalgamation configurations of Botan are included in the source tree to make getting started with development easier. Android requires a small patch to implement to_string, which is missing from the NDK STL. The patch, android_to_string.hpp, is included by botan_all.h from src/android/ in the project tree. The configuration commands used to generate a minimal Botan amalgamation follows for each platform:

###Android (ARM)
python configure.py --cpu=armv5te --via-amalgamation --no-autoload --enable-modules=aes,serpent,serpent_simd,gcm,eax,keccak,pbkdf2,locking_allocator,dev_random,kdf2,aead_filt

###Linux GCC x86_64
python configure.py --via-amalgamation --no-autoload --enable-modules=aes,serpent,serpent_simd,gcm,eax,keccak,pbkdf2,locking_allocator,dev_random,kdf2,aead_filt

###Mac OS X clang
python configure.py --cc=clang --via-amalgamation --no-autoload --enable-modules=aes,serpent,serpent_simd,gcm,eax,keccak,pbkdf2,locking_allocator,dev_random,kdf2,aead_filt

###iOS clang
python configure.py --cpu=armv7 --cc=clang --via-amalgamation --no-autoload --enable-modules=aes,serpent,serpent_simd,gcm,eax,keccak,pbkdf2,locking_allocator,dev_random,kdf2,aead_filt

###Windows MinGW-w64 GCC x64
python configure.py --cpu=x64 --cc=gcc --os=mingw --via-amalgamation --no-autoload --enable-modules=aes,serpent,serpent_simd,gcm,eax,keccak,pbkdf2,win32_stats,kdf2,aead_filt

###Windows MinGW GCC x86
python configure.py --cpu=x86 --cc=gcc --os=mingw --via-amalgamation --no-autoload --enable-modules=aes,serpent,gcm,eax,keccak,pbkdf2,win32_stats,kdf2,aead_filt

##Contact

You can contact the project creator via email at andrewdolby@gmail.com
