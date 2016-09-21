#!/bin/bash

set -o errexit -o nounset

# Update platform
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install libfontconfig1
sudo apt-get install libfontconfig1-dev
sudo apt-get install libfreetype6-dev
sudo apt-get install libx11-dev
sudo apt-get install libxext-dev
sudo apt-get install libxfixes-dev
sudo apt-get install libxi-dev
sudo apt-get install libxrender-dev
sudo apt-get install libxcb1-dev
sudo apt-get install libx11-xcb-dev
sudo apt-get install libxcb-glx0-dev
sudo apt-get install mesa-common-dev
sudo apt-get install libglu1-mesa-dev -y

# Capture build directory
project=$(pwd)

# Build Qt
cd /tmp/
wget https://download.qt.io/official_releases/qt/5.7/5.7.0/single/qt-everywhere-opensource-src-5.7.0.tar.gz
gunzip qt-everywhere-opensource-src-5.7.0.tar.gz
tar xvf qt-everywhere-opensource-src-5.7.0.tar.gz
cd qt-everywhere-opensource-src-5.7.0
sudo chmod +x configure
./configure

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
ls
7z a kryvos_${TRAVIS_TAG}_linux_x86_64.zip "Kryvos" "Release Notes" "README.md" "LICENSE" "Botan License" "Qt License"

exit 0
