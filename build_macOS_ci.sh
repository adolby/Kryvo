#!/bin/bash

set -o errexit -o nounset

PROJECT_DIR="${SOURCE_DIR}"
QT_PATH="${Qt6_DIR}"
KRYVO_VERSION="${KRYVO_VERSION:-dev}"

# Output macOS version
sw_vers

# Update platform
echo "Updating platform..."
brew update
brew install p7zip
# npm install -g appdmeg

# Add Qt binaries to path
echo "Adding Qt binaries to path..."
PATH="${QT_PATH}/bin/:${PATH}"

# Check qmake version
qmake --version

cd "${PROJECT_DIR}"

# Clean build directory
rm -rf "${PROJECT_DIR}/build/macOS/"

mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/widgets/"
mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/widgets/moc"
mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/widgets/qrc"
mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/widgets/obj"
mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/quick/"
mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/quick/moc"
mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/quick/qrc"
mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/quick/obj"
mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/lib/"
mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/lib/zlib/"
mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/test/"
mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/Kryvo/"

# Build Kryvo
echo "Building Kryvo..."

if [ -f "${PROJECT_DIR}/Makefile" ]; then
  make distclean
fi

qmake -makefile CONFIG+=release OPENSSL_PATH=/usr/local/opt/openssl@3
make

echo "Skipping tests..."

# Copy Qt dependencies for test app
# echo "Copying Qt dependencies to test app..."
# cd "${PROJECT_DIR}/build/macOS/clang/x86_64/release/test/"
# macdeployqt tests.app

# Copy plugins to test app
# echo "Copying plugins for test app..."
# mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/test/tests.app/Contents/PlugIns/cryptography/botan/"
# cd "${PROJECT_DIR}/build/macOS/clang/x86_64/release/test/tests.app/Contents/PlugIns/cryptography/botan/"
# cp "${PROJECT_DIR}/build/macOS/clang/x86_64/release/plugins/cryptography/botan/libbotan.dylib" libbotan.dylib

# Copy test data
# echo "Copying test data archive..."
# cd "${PROJECT_DIR}/build/macOS/clang/x86_64/release/test/tests.app/Contents/MacOS/"
# cp "${PROJECT_DIR}/src/tests/data/test-data.zip" test-data.zip
#
# echo "Extracting test data..."
# 7z e test-data.zip &>/dev/null

# Run tests
# echo "Running tests..."
# chmod +x tests
# ./tests

# Copy plugins to app
# echo "Copy plugins to app..."
# mkdir -p "${PROJECT_DIR}/build/macOS/clang/x86_64/release/widgets/Kryvo.app/Contents/PlugIns/cryptography/botan/"
# cd "${PROJECT_DIR}/build/macOS/clang/x86_64/release/widgets/Kryvo.app/Contents/PlugIns/cryptography/botan/"
# cp "${PROJECT_DIR}/build/macOS/clang/x86_64/release/plugins/cryptography/botan/libbotan.dylib" libbotan.dylib

# Package Kryvo
echo "Packaging..."

echo "Copying app to packaging directory..."
cp -a "${PROJECT_DIR}/build/macOS/clang/x86_64/release/widgets/." "${PROJECT_DIR}/build/macOS/clang/x86_64/release/Kryvo/"

cd "${PROJECT_DIR}/build/macOS/clang/x86_64/release/Kryvo/"

rm -rf moc
rm -rf obj
rm -rf qrc

echo "Copying Qt dependencies and creating dmg archive..."
macdeployqt Kryvo.app -dmg

mv Kryvo.dmg "Kryvo_${KRYVO_VERSION}.dmg"
# appdmg json-path Kryvo_${TRAVIS_TAG}.dmg

cp -a "${PROJECT_DIR}/Release Notes" "Release Notes"
cp -a "${PROJECT_DIR}/README.md" "README.md"
cp -a "${PROJECT_DIR}/LICENSE" "LICENSE"
cp -a "${PROJECT_DIR}/Botan License" "Botan License"
cp -a "${PROJECT_DIR}/Qt License" "Qt License"
mkdir themes
cp -a "${PROJECT_DIR}/resources/stylesheets/kryvo.qss" "themes/kryvo.qss"

echo "Packaging archive..."
tar czvf "kryvo_${KRYVO_VERSION}_macos.tar.gz" "Kryvo_${KRYVO_VERSION}.dmg" "Release Notes" "README.md" "LICENSE" "Botan License" "Qt License"

cd "${PROJECT_DIR}/build/macOS/clang/x86_64/release/"

mv "Kryvo/kryvo_${KRYVO_VERSION}_macos.tar.gz" "kryvo_${KRYVO_VERSION}_macos.tar.gz"

echo "Done!"

exit 0
