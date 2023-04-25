#!/bin/bash

set -o errexit -o nounset

PROJECT_DIR="${SOURCE_DIR}"
QT_PATH="${Qt6_DIR}"
QT_TOOLS="${IQTA_TOOLS}"
KRYVO_VERSION="${KRYVO_VERSION:-dev}"

# Add Qt binaries to path
echo "Adding Qt binaries to path..."
PATH="${QT_PATH}/bin/:${QT_TOOLS}/QtInstallerFramework/4.5/bin/:${PATH}"

# Check qmake version
qmake --version

cd "${PROJECT_DIR}"

# Clean build directory
rm -rf "${PROJECT_DIR}/build/linux/"

mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/widgets/"
mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/widgets/moc"
mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/widgets/qrc"
mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/widgets/obj"
mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/quick/"
mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/quick/moc"
mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/quick/qrc"
mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/quick/obj"
mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/lib/"
mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/lib/zlib/"
mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/test/"
mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/Kryvo/"

# Build Kryvo
echo "Building Kryvo..."

if [ -f "${PROJECT_DIR}/Makefile" ]; then
  make distclean
fi

qmake -makefile -spec linux-g++ CONFIG+=release OPENSSL_PATH=/usr/lib/x86_64-linux-gnu
make

# Copy Qt dependencies for test app
echo "Copying Qt dependencies to test app..."
cd "${PROJECT_DIR}/build/linux/gcc/x86_64/release/test/"

cp -a "${QT_PATH}/lib/libicui18n.so.56.1" "libicui18n.so.56.1"
ln -s "libicui18n.so.56.1" "libicui18n.so"
ln -s "libicui18n.so.56.1" "libicui18n.so.56"

cp -a "${QT_PATH}/lib/libicuuc.so.56.1" "libicuuc.so.56.1"
ln -s "libicuuc.so.56.1" "libicuuc.so"
ln -s "libicuuc.so.56.1" "libicuuc.so.56"

cp -a "${QT_PATH}/lib/libicudata.so.56.1" "libicudata.so.56.1"
ln -s "libicudata.so.56.1" "libicudata.so"
ln -s "libicudata.so.56.1" "libicudata.so.56"

cp -a "${QT_PATH}/lib/libQt6Core.so.6.4.2" "libQt6Core.so.6.4.2"
ln -s "libQt6Core.so.6.4.2" "libQtCore.so"
ln -s "libQt6Core.so.6.4.2" "libQtCore.so.6"

cp -a "${QT_PATH}/lib/libQt6Test.so.6.4.2" "libQt6Test.so.6.4.2"
ln -s "libQt6Test.so.6.4.2" "libQtTest.so"
ln -s "libQt6Test.so.6.4.2" "libQtTest.so.6"

cp -a "${QT_TOOLS}\OpenSSLv3\linux\bin\libcrypto-3-x64.so" "libcrypto-3-x64.so"
cp -a "${QT_TOOLS}\OpenSSLv3\linux\bin\libssl-3-x64.so" "libssl-3-x64.so"

# Copy plugins for test app
# echo "Copying plugins for test app..."
# mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/test/plugins/cryptography/botan/"
# cd "${PROJECT_DIR}/build/linux/gcc/x86_64/release/test/plugins/cryptography/botan/"
# cp "${PROJECT_DIR}/build/linux/gcc/x86_64/release/plugins/cryptography/botan/libbotan.so" libbotan.so

# Copy test data
echo "Copying test data archive..."
cd "${PROJECT_DIR}/build/linux/gcc/x86_64/release/test/"
cp "${PROJECT_DIR}/src/tests/data/test-data.zip" test-data.zip

echo "Extracting test data..."
7z e test-data.zip -aos &> /dev/null

# Run tests
echo "Running tests..."
chmod +x tests
./tests

# Copy plugins for app
# echo "Copy plugins to app..."
# mkdir -p "${PROJECT_DIR}/build/linux/gcc/x86_64/release/widgets/plugins/cryptography/botan/"
# cd "${PROJECT_DIR}/build/linux/gcc/x86_64/release/widgets/plugins/cryptography/botan/"
# cp "${PROJECT_DIR}/build/linux/gcc/x86_64/release/plugins/cryptography/botan/libbotan.so" libbotan.so

# Package Kryvo
echo "Packaging..."

echo "Copying app to packaging directory..."
cp -a "${PROJECT_DIR}/build/linux/gcc/x86_64/release/widgets/." "${PROJECT_DIR}/build/linux/gcc/x86_64/release/Kryvo/"

cd "${PROJECT_DIR}/build/linux/gcc/x86_64/release/Kryvo/"

rm -rf moc
rm -rf obj
rm -rf qrc

echo "Copying Qt dependencies..."

cp -a "${QT_PATH}/lib/libicui18n.so.56.1" "libicui18n.so.56.1"
ln -s "libicui18n.so.56.1" "libicui18n.so"
ln -s "libicui18n.so.56.1" "libicui18n.so.56"

cp -a "${QT_PATH}/lib/libicuuc.so.56.1" "libicuuc.so.56.1"
ln -s "libicuuc.so.56.1" "libicuuc.so"
ln -s "libicuuc.so.56.1" "libicuuc.so.56"

cp -a "${QT_PATH}/lib/libicudata.so.56.1" "libicudata.so.56.1"
ln -s "libicudata.so.56.1" "libicudata.so"
ln -s "libicudata.so.56.1" "libicudata.so.56"

mkdir platforms
cp -a "${QT_PATH}/plugins/platforms/libqxcb.so" "platforms/libqxcb.so"
cp -a "${QT_PATH}/plugins/platforms/libqminimal.so" "platforms/libqminimal.so"

cp -a "${QT_PATH}/lib/libQt6Core.so.6.4.2" "libQt6Core.so.6.4.2"
ln -s "libQt6Core.so.6.4.2" "libQt6Core.so"
ln -s "libQt6Core.so.6.4.2" "libQt6Core.so.6"

cp -a "${QT_PATH}/lib/libQt6Gui.so.6.4.2" "libQt6Gui.so.6.4.2"
ln -s "libQt6Gui.so.6.4.2" "libQt6Gui.so"
ln -s "libQt6Gui.so.6.4.2" "libQt6Gui.so.6"

cp -a "${QT_PATH}/lib/libQt6OpenGL.so.6.4.2" "libQt6OpenGL.so.6.4.2"
ln -s "libQt6OpenGL.so.6.4.2" "libQt6OpenGL.so"
ln -s "libQt6OpenGL.so.6.4.2" "libQt6OpenGL.so.6"

cp -a "${QT_PATH}/lib/libQt6Svg.so.6.4.2" "libQt6Svg.so.6.4.2"
ln -s "libQt6Svg.so.6.4.2" "libQt6Svg.so"
ln -s "libQt6Svg.so.6.4.2" "libQt6Svg.so.6"

cp -a "${QT_PATH}/lib/libQt6DBus.so.6.4.2" "libQt6DBus.so.6.4.2"
ln -s "libQt6DBus.so.6.4.2" "libQt6DBus.so"
ln -s "libQt6DBus.so.6.4.2" "libQt6DBus.so.6"

cp -a "${QT_PATH}/lib/libQt6XcbQpa.so.6.4.2" "libQt6XcbQpa.so.6.4.2"
ln -s "libQt6XcbQpa.so.6.4.2" "libQt6XcbQpa.so"
ln -s "libQt6XcbQpa.so.6.4.2" "libQt6XcbQpa.so.6"

cp -a "${QT_PATH}/lib/libQt6Widgets.so.6.4.2" "libQt6Widgets.so.6.4.2"
ln -s "libQt6Widgets.so.6.4.2" "libQt6Widgets.so"
ln -s "libQt6Widgets.so.6.4.2" "libQt6Widgets.so.6"

chrpath -r \$ORIGIN/.. platforms/libqxcb.so
chrpath -r \$ORIGIN/.. platforms/libqminimal.so

cp -a "${QT_TOOLS}\OpenSSLv3\linux\bin\libcrypto-3-x64.so" "libcrypto-3-x64.so"
cp -a "${QT_TOOLS}\OpenSSLv3\linux\bin\libssl-3-x64.so" "libssl-3-x64.so"

cp -a "${PROJECT_DIR}/Release Notes" "Release Notes"
cp -a "${PROJECT_DIR}/README.md" "README.md"
cp -a "${PROJECT_DIR}/LICENSE" "LICENSE"
cp -a "${PROJECT_DIR}/Botan License" "Botan License"
cp -a "${PROJECT_DIR}/Qt License" "Qt License"
mkdir themes
cp -a "${PROJECT_DIR}/resources/stylesheets/kryvo.qss" "themes/kryvo.qss"

echo "Copying files for installer..."
mkdir -p "${PROJECT_DIR}/installer/linux/packages/app.kryvo/data/"
cp -a * "${PROJECT_DIR}/installer/linux/packages/app.kryvo/data/"

echo "Packaging portable archive..."
cd ..
tar czvf kryvo_${KRYVO_VERSION}_linux_x86_64_portable.tar.gz Kryvo

echo "Creating installer..."
cd "${PROJECT_DIR}/installer/linux/"
binarycreator --offline-only -c config/config.xml -p packages kryvo_${KRYVO_VERSION}_linux_x86_64_installer

echo "Done!"

exit 0
