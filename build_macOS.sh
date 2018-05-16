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
brew install qt

# Add Qt binaries to path
PATH=/usr/local/opt/qt/bin/:${PATH}

# Get Botan
# echo "Installing Botan..."
# wget https://github.com/randombit/botan/archive/1.11.32.zip
# 7z x 1.11.32.zip &>/dev/null
# chmod -R +x /usr/local/botan-1.11.32/
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

# Build Kryvo
echo "Building Kryvo..."
cd ${project_dir}/src/
qmake -config release
make

# Copy dependencies for test app
cd ${project_dir}/build/macOS/clang/x86_64/release/test/
macdeployqt CryptoTests.app

# Copy test data
echo "Copying test data..."
cd ${project_dir}/build/macOS/clang/x86_64/release/test/CryptoTests.app/Contents/MacOS/
cp ${project_dir}/src/tests/data/test-data.zip test-data.zip
7z x test-data.zip &> /dev/null

# Run tests
echo "Running tests..."
chmod +x CryptoTests
./CryptoTests

# Package Kryvo
echo "Packaging..."
cd ${project_dir}/build/macOS/clang/x86_64/release/Kryvo/

rm -rf moc
rm -rf obj
rm -rf qrc

echo "Creating dmg archive..."
macdeployqt Kryvo.app -dmg
mv Kryvo.dmg "Kryvo_${TAG_NAME}.dmg"
# appdmg json-path Kryvo_${TRAVIS_TAG}.dmg

cp "${project_dir}/Release Notes" "Release Notes"
cp "${project_dir}/README.md" "README.md"
cp "${project_dir}/LICENSE" "LICENSE"
cp "${project_dir}/Botan License" "Botan License"
cp "${project_dir}/Qt License" "Qt License"
# mkdir themes
# cp "${project_dir}/resources/stylesheets/kryvo.qss" "themes/kryvo.qss"

echo "Packaging zip archive..."
7z a kryvo_${TAG_NAME}_macos.zip "Kryvo_${TAG_NAME}.dmg" "Release Notes" "README.md" "LICENSE" "Botan License" "Qt License"

echo "Done!"

exit 0
