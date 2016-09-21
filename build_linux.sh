#!/bin/bash

set -o errexit -o nounset

sudo add-apt-repository ppa:beineri/opt-qt57-trusty -y
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install build-essential
sudo apt-get install libfontconfig1
sudo apt-get install mesa-common-dev
sudo apt-get install libglu1-mesa-dev -y

cd src

/usr/local/opt/qt5/bin/qmake -config release
make

cd tests
/usr/local/opt/qt5/bin/qmake -config release
make
cd ../../build/linux/gcc/x86_64/release/test/
chmod +x CryptoTests
./CryptoTests

cd ..
cp "../../../../../Release Notes" "Release Notes"
cp "../../../../../README.md" "README.md"
cp "../../../../../LICENSE" "LICENSE"
cp "../../../../../Botan License" "Botan License"
cp "../../../../../Qt License" "Qt License"
ls
7z a kryvos_${TRAVIS_TAG}_linux_x86_64.zip "Kryvos" "Release Notes" "README.md" "LICENSE" "Botan License" "Qt License"

exit 0
