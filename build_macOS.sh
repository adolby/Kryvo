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
echo "Adding Qt binaries to path..."
PATH="usr/local/opt/qt/bin/:${PATH}"

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

cd "${project_dir}"

# Clean build directory
rm -rf "${project_dir}/build/macOS/"

mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/widgets/"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/widgets/moc"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/widgets/qrc"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/widgets/obj"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/quick/"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/lib/"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/lib/zlib/"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/test/"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/Kryvo/"

# Build Kryvo
echo "Building Kryvo..."

if [ -f "${project_dir}/Makefile" ]; then
  make distclean
fi

qmake CONFIG+=release
make

# Copy Qt dependencies for test app
echo "Copying Qt dependencies to test app..."
cd "${project_dir}/build/macOS/clang/x86_64/release/test/"
macdeployqt tests.app

# Copy plugins to test app
echo "Copying plugins for test app..."
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/test/tests.app/Contents/PlugIns/cryptography/botan/"
cd "${project_dir}/build/macOS/clang/x86_64/release/test/tests.app/Contents/PlugIns/cryptography/botan/"
cp "${project_dir}/build/macOS/clang/x86_64/release/plugins/cryptography/botan/libbotan.dylib" libbotan.dylib

# Copy test data
echo "Copying test data archive..."
cd "${project_dir}/build/macOS/clang/x86_64/release/test/tests.app/Contents/MacOS/"
cp "${project_dir}/src/tests/data/test-data.zip" test-data.zip

echo "Extracting test data..."
7z e test-data.zip &>/dev/null

# Run tests
echo "Skipping tests..."
# chmod +x tests
# ./tests

# Copy plugins to app
echo "Copy plugins to app..."
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/Kryvo/Kryvo.app/Contents/PlugIns/cryptography/botan/"
cd "${project_dir}/build/macOS/clang/x86_64/release/widgets/Kryvo.app/Contents/PlugIns/cryptography/botan/"
cp "${project_dir}/build/macOS/clang/x86_64/release/plugins/cryptography/botan/libbotan.dylib" libbotan.dylib

# Package Kryvo
echo "Packaging..."

echo "Copying app to packaging directory..."
cp -r "${project_dir}/build/macOS/clang/x86_64/release/widgets/." "${project_dir}/build/macOS/clang/x86_64/release/Kryvo/"

cd "${project_dir}/build/macOS/clang/x86_64/release/Kryvo/"

rm -rf moc
rm -rf obj
rm -rf qrc

echo "Copying Qt dependencies and creating dmg archive..."
macdeployqt Kryvo.app -dmg

TAG_NAME="${TAG_NAME:-dev}"

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
