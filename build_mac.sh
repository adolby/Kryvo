#!/bin/bash

set -o errexit -o nounset

# Update platform
brew update
brew install qt5
brew install p7zip
npm install -g appdmg
chmod -R 755 /usr/local/opt/qt5/*

# Build
cd src
/usr/local/opt/qt5/bin/qmake -config release
make -j2

# Run tests
cd tests
/usr/local/opt/qt5/bin/qmake -config release
make
cd ../../build/macx/clang/x86_64/release/test/
open CryptoTests.app

# Package
cd ..
/usr/local/opt/qt5/bin/macdeployqt Kryvos.app -dmg
mv Kryvos.dmg "Kryvos_${TRAVIS_TAG}.dmg"
# appdmg json-path Kryvos_${TRAVIS_TAG}.dmg
cp "../../../../../Release Notes" "Release Notes"
cp "../../../../../README.md" "README.md"
cp "../../../../../LICENSE" "LICENSE"
cp "../../../../../Botan License" "Botan License"
cp "../../../../../Qt License" "Qt License"
7z a kryvos_${TRAVIS_TAG}_macos.zip "Kryvos_${TRAVIS_TAG}.dmg" "Release Notes" "README.md" "LICENSE" "Botan License" "Qt License"

exit 0
