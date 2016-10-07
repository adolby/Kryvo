#!/bin/bash

set -o errexit -o nounset

project_dir=$(pwd)

# Output macOS version
sw_vers

# Update platform
echo "Updating platform..."
brew update
brew install p7zip
npm install -g appdmg

# Get Qt
echo "Installing Qt..."
cd /usr/local/
sudo wget https://github.com/adolby/qt-more-builds/releases/download/5.7/qt-opensource-5.7.0-x86_64-macos-clang.zip
sudo 7z x qt-opensource-5.7.0-x86_64-macos-clang.zip &>/dev/null
sudo chmod -R +x /usr/local/Qt-5.7.0/bin/

# Add Qt binaries to path
PATH=/usr/local/Qt-5.7.0/bin/:${PATH}

# Get Botan
# echo "Installing Botan..."
# sudo wget https://github.com/randombit/botan/archive/1.11.32.zip
# sudo 7z x 1.11.32.zip &>/dev/null
# sudo chmod -R +x /usr/local/botan-1.11.32/
# cd /usr/local/botan-1.11.32/
# ./configure.py --cc=clang --amalgamation --disable-shared --with-zlib
# cp botan_all_aesni.cpp ${project_dir}/src/cryptography/botan/macOS/clang/x86_64/botan_all_aesni.cpp
# cp botan_all_avx2.cpp ${project_dir}/src/cryptography/botan/macOS/clang/x86_64/botan_all_avx2.cpp
# cp botan_all_internal.h ${project_dir}/src/cryptography/botan/macOS/clang/x86_64/botan_all_internal.h
# cp botan_all_rdrand.cpp ${project_dir}/src/cryptography/botan/macOS/clang/x86_64/botan_all_rdrand.cpp
# cp botan_all_rdseed.cpp ${project_dir}/src/cryptography/botan/macOS/clang/x86_64/botan_all_rdseed.cpp
# cp botan_all_ssse3.cpp ${project_dir}/src/cryptography/botan/macOS/clang/x86_64/botan_all_ssse3.cpp
# cp botan_all.cpp ${project_dir}/src/cryptography/botan/macOS/clang/x86_64/botan_all.cpp
# cp botan_all.h ${project_dir}/src/cryptography/botan/macOS/clang/x86_64/botan_all.h

# Create temporary symlink for Xcode8 compatibility
cd /Applications/Xcode.app/Contents/Developer/usr/bin/
sudo ln -s xcodebuild xcrun

# Build Kryvos
echo "Building Kryvos..."
cd ${project_dir}
qmake -config release
make

# Build tests
echo "Building tests..."
cd ${project_dir}/tests/
qmake -config release
make

# Copy test data
echo "Copying test data..."
cd ${project_dir}/build/macOS/clang/x86_64/release/test/CryptoTests.app/Contents/MacOS/
cp ${project_dir}/tests/data/test-data.zip test-data.zip
7z x test-data.zip &>/dev/null

# Run tests
echo "Running tests..."
sudo chmod +x CryptoTests
# Disable running tests until Travis adds support for macOS 10.12
# ./CryptoTests

# Package Kryvos
echo "Packaging..."
cd ${project_dir}/build/macOS/clang/x86_64/release/

rm -rf moc
rm -rf obj
rm -rf qrc

echo "Creating dmg archive..."
macdeployqt Kryvos.app -dmg
mv Kryvos.dmg "Kryvos_${TAG_NAME}.dmg"
# appdmg json-path Kryvos_${TRAVIS_TAG}.dmg

cp "${project_dir}/Release Notes" "Release Notes"
cp "${project_dir}/README.md" "README.md"
cp "${project_dir}/LICENSE" "LICENSE"
cp "${project_dir}/Botan License" "Botan License"
cp "${project_dir}/Qt License" "Qt License"
# mkdir themes
# cp "${project_dir}/resources/stylesheets/kryvos.qss" "themes/kryvos.qss"

echo "Packaging zip archive..."
7z a kryvos_${TAG_NAME}_macos.zip "Kryvos_${TAG_NAME}.dmg" "Release Notes" "README.md" "LICENSE" "Botan License" "Qt License"

echo "Done!"

exit 0
