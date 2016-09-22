#!/bin/bash

set -o errexit -o nounset

# Update platform
sudo apt-get update
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-6 100
gcc --version
g++ --version
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" upgrade
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libfontconfig1
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libfontconfig1-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libfreetype6-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libx11-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxext-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxfixes-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxi-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxrender-dev
sudo apt-get -y -qq -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxcb1-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libx11-xcb-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxcb-glx0-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install mesa-common-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libglu1-mesa-dev

# Capture build directory
project=$(pwd)

# Build Qt
cd /tmp/
wget https://download.qt.io/official_releases/qt/5.7/5.7.0/single/qt-everywhere-opensource-src-5.7.0.tar.gz
gunzip qt-everywhere-opensource-src-5.7.0.tar.gz
tar xf qt-everywhere-opensource-src-5.7.0.tar

cd qt-everywhere-opensource-src-5.7.0
sudo chmod +x configure
./configure -platform linux-g++-64 -opensource -confirm-license -release -c++std c++14 -shared -largefile -no-qml-debug -qt-libpng -qt-libjpeg -qt-doubleconversion -qt-harfbuzz -qt-xcb -openssl -qt-pcre -skip qtwebengine -nomake examples

make
make install

# Build
cd ${project}/src/
/usr/local/Qt-5.7.0/bin/qmake -config release
make

# Run tests
cd tests
/usr/local/Qt-5.7.0/bin/qmake -config release
make
cd ../../build/linux/gcc/x86_64/release/test/
sudo chmod +x CryptoTests
./CryptoTests

# Package
cd ..
cp "../../../../../Release Notes" "Release Notes"
cp "../../../../../README.md" "README.md"
cp "../../../../../LICENSE" "LICENSE"
cp "../../../../../Botan License" "Botan License"
cp "../../../../../Qt License" "Qt License"
7z a kryvos_${TRAVIS_TAG}_linux_x86_64.zip "Kryvos" "Release Notes" "README.md" "LICENSE" "Botan License" "Qt License"

exit 0
