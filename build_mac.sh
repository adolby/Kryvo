#!/bin/bash

set -o errexit -o nounset

project_dir=$(pwd)

# Update platform
echo "Updating platform..."
brew update
brew install p7zip
npm install -g appdmg
chmod -R 755 /usr/local/opt/qt5/*

# Get Qt
echo "Installing Qt..."
cd /usr/local/
sudo wget https://github.com/adolby/qt-more-builds/releases/download/5.7/qt-opensource-5.7.0-x86_64-macos-clang.zip
sudo 7z x qt-opensource-5.7.0-x86_64-macos-clang.zip &>/dev/null
sudo chmod -R +x /usr/local/Qt-5.7.0/bin/

# Build
echo "Building Kryvos..."
cd ${project_dir}/src/
/usr/local/Qt-5.7.0/bin/qmake -config release
make

# Run tests
echo "Building tests..."
cd ${project_dir}/src/tests/
/usr/local/Qt-5.7.0/bin/qmake -config release
make

echo "Running tests..."
cd ${project_dir}/build/macx/clang/x86_64/release/test/
open CryptoTests.app

# Package
echo "Packaging..."
cd ${project_dir}/build/macx/clang/x86_64/release/
rm -rf moc
rm -rf obj
rm -rf qrc

echo "Creating dmg archive..."
/usr/local/Qt-5.7.0/bin/macdeployqt Kryvos.app -dmg
mv Kryvos.dmg "Kryvos_${TAG_NAME}.dmg"
# appdmg json-path Kryvos_${TRAVIS_TAG}.dmg

cp "${project_dir}/Release Notes" "Release Notes"
cp "${project_dir}/README.md" "README.md"
cp "${project_dir}/LICENSE" "LICENSE"
cp "${project_dir}/Botan License" "Botan License"
cp "${project_dir}/Qt License" "Qt License"
mkdir themes
cp "${project_dir}/src/resources/stylesheets/kryvos.qss" "themes/kryvos.qss"

echo "Packaging zip archive..."
7z a kryvos_${TAG_NAME}_macos.zip "Kryvos_${TAG_NAME}.dmg" "Release Notes" "README.md" "LICENSE" "Botan License" "Qt License"

echo "Done!"

exit 0
