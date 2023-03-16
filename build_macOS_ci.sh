#!/bin/bash

set -o errexit -o nounset

project_dir="${SOURCE_DIR}"
qt_path="${Qt6_DIR}"
tag_name="${TAG_NAME:-dev}"

# Output macOS version
sw_vers

# Update platform
echo "Updating platform..."
brew update
brew install p7zip
# npm install -g appdmeg

# Add Qt binaries to path
echo "Adding Qt binaries to path..."
PATH="${qt_path}/bin/:${PATH}"

# Check qmake version
qmake --version

cd "${project_dir}"

# Clean build directory
rm -rf "${project_dir}/build/macOS/"

mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/widgets/"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/widgets/moc"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/widgets/qrc"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/widgets/obj"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/quick/"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/quick/moc"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/quick/qrc"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/quick/obj"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/lib/"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/lib/zlib/"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/test/"
mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/Kryvo/"

# Build Kryvo
echo "Building Kryvo..."

if [ -f "${project_dir}/Makefile" ]; then
  make distclean
fi

qmake -makefile CONFIG+=release
make

echo "Skipping tests..."

# Copy Qt dependencies for test app
# echo "Copying Qt dependencies to test app..."
# cd "${project_dir}/build/macOS/clang/x86_64/release/test/"
# macdeployqt tests.app

# Copy plugins to test app
# echo "Copying plugins for test app..."
# mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/test/tests.app/Contents/PlugIns/cryptography/botan/"
# cd "${project_dir}/build/macOS/clang/x86_64/release/test/tests.app/Contents/PlugIns/cryptography/botan/"
# cp "${project_dir}/build/macOS/clang/x86_64/release/plugins/cryptography/botan/libbotan.dylib" libbotan.dylib

# Copy test data
# echo "Copying test data archive..."
# cd "${project_dir}/build/macOS/clang/x86_64/release/test/tests.app/Contents/MacOS/"
# cp "${project_dir}/src/tests/data/test-data.zip" test-data.zip
#
# echo "Extracting test data..."
# 7z e test-data.zip &>/dev/null

# Run tests
# echo "Running tests..."
# chmod +x tests
# ./tests

# Copy plugins to app
# echo "Copy plugins to app..."
# mkdir -p "${project_dir}/build/macOS/clang/x86_64/release/widgets/Kryvo.app/Contents/PlugIns/cryptography/botan/"
# cd "${project_dir}/build/macOS/clang/x86_64/release/widgets/Kryvo.app/Contents/PlugIns/cryptography/botan/"
# cp "${project_dir}/build/macOS/clang/x86_64/release/plugins/cryptography/botan/libbotan.dylib" libbotan.dylib

# Package Kryvo
echo "Packaging..."

echo "Copying app to packaging directory..."
cp -a "${project_dir}/build/macOS/clang/x86_64/release/widgets/." "${project_dir}/build/macOS/clang/x86_64/release/Kryvo/"

cd "${project_dir}/build/macOS/clang/x86_64/release/Kryvo/"

rm -rf moc
rm -rf obj
rm -rf qrc

echo "Copying Qt dependencies and creating dmg archive..."
macdeployqt Kryvo.app -dmg

mv Kryvo.dmg "Kryvo_${tag_name}.dmg"
# appdmg json-path Kryvo_${TRAVIS_TAG}.dmg

cp -a "${project_dir}/Release Notes" "Release Notes"
cp -a "${project_dir}/README.md" "README.md"
cp -a "${project_dir}/LICENSE" "LICENSE"
cp -a "${project_dir}/Botan License" "Botan License"
cp -a "${project_dir}/Qt License" "Qt License"
mkdir themes
cp -a "${project_dir}/resources/stylesheets/kryvo.qss" "themes/kryvo.qss"

echo "Packaging archive..."
tar czvf "kryvo_${tag_name}_macos.tar.gz" "Kryvo_${tag_name}.dmg" "Release Notes" "README.md" "LICENSE" "Botan License" "Qt License"

cd "${project_dir}/build/macOS/clang/x86_64/release/"

mv "Kryvo/kryvo_${tag_name}_macos.tar.gz" "kryvo_${tag_name}_macos.tar.gz"

echo "Done!"

exit 0
