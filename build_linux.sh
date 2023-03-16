#!/bin/bash

set -o errexit -o nounset

project_dir="${SOURCE_DIR}"
qt_path="${Qt6_DIR}"
qt_tools="${IQTA_TOOLS}"
tag_name="${TAG_NAME:-dev}"

# Add Qt binaries to path
echo "Adding Qt binaries to path..."
PATH="${qt_path}/bin/:${qt_tools}/QtInstallerFramework/4.5/bin/:${PATH}"

# Check qmake version
qmake --version

# Get Botan
# echo "Installing Botan..."
# wget https://github.com/randombit/botan/archive/1.11.32.zip
# 7z x 1.11.32.zip &>/dev/null
# chmod -R +x /usr/local/botan-1.11.32/
# cd /usr/local/botan-1.11.32/
# ./configure.py --cc=clang --amalgamation --disable-shared --with-zlib
# cp botan_all_aesni.cpp ${project_dir}/src/cryptography/botan/linux/gcc/x86_64/botan_all_aesni.cpp
# cp botan_all_avx2.cpp ${project_dir}/src/cryptography/botan/linux/gcc/x86_64/botan_all_avx2.cpp
# cp botan_all_internal.h ${project_dir}/src/cryptography/botan/linux/gcc/x86_64/botan_all_internal.h
# cp botan_all_rdrand.cpp ${project_dir}/src/cryptography/botan/linux/gcc/x86_64/botan_all_rdrand.cpp
# cp botan_all_rdseed.cpp ${project_dir}/src/cryptography/botan/linux/gcc/x86_64/botan_all_rdseed.cpp
# cp botan_all_ssse3.cpp ${project_dir}/src/cryptography/botan/linux/gcc/x86_64/botan_all_ssse3.cpp
# cp botan_all.cpp ${project_dir}/src/cryptography/botan/linux/gcc/x86_64/botan_all.cpp
# cp botan_all.h ${project_dir}/src/cryptography/botan/linux/gcc/x86_64/botan_all.h

cd "${project_dir}"

# Clean build directory
rm -rf "${project_dir}/build/linux/"

mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/widgets/"
mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/widgets/moc"
mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/widgets/qrc"
mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/widgets/obj"
mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/quick/"
mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/quick/moc"
mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/quick/qrc"
mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/quick/obj"
mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/lib/"
mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/lib/zlib/"
mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/test/"
mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/Kryvo/"

# Build Kryvo
echo "Building Kryvo..."

if [ -f "${project_dir}/Makefile" ]; then
  make distclean
fi

qmake -makefile -spec linux-g++ CONFIG+=release
make

# Copy Qt dependencies for test app
echo "Copying Qt dependencies to test app..."
cd "${project_dir}/build/linux/gcc/x86_64/release/test/"

cp -P "${qt_path}/lib/libicui18n.so" "libicui18n.so"
cp -P "${qt_path}/lib/libicui18n.so.56" "libicui18n.so.56"
cp "${qt_path}/lib/libicui18n.so.56.1" "libicui18n.so.56.1"
cp -P "${qt_path}/lib/libicuuc.so" "libicuuc.so"
cp -P "${qt_path}/lib/libicuuc.so.56" "libicuuc.so.56"
cp "${qt_path}/lib/libicuuc.so.56.1" "libicuuc.so.56.1"
cp -P "${qt_path}/lib/libicudata.so" "libicudata.so"
cp -P "${qt_path}/lib/libicudata.so.56" "libicudata.so.56"
cp "${qt_path}/lib/libicudata.so.56.1" "libicudata.so.56.1"

cp -P "${qt_path}/lib/libQt6Core.so" "libQt6Core.so"
cp -P "${qt_path}/lib/libQt6Core.so.6" "libQt6Core.so.6"
cp "${qt_path}/lib/libQt6Core.so.6.4.2" "libQt6Core.so.6.4.2"
cp -P "${qt_path}/lib/libQt6Test.so" "libQt6Test.so"
cp -P "${qt_path}/lib/libQt6Test.so.6" "libQt6Test.so.6"
cp "${qt_path}/lib/libQt6Test.so.6.4.2" "libQt6Test.so.6.4.2"

# Copy plugins for test app
# echo "Copying plugins for test app..."
# mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/test/plugins/cryptography/botan/"
# cd "${project_dir}/build/linux/gcc/x86_64/release/test/plugins/cryptography/botan/"
# cp "${project_dir}/build/linux/gcc/x86_64/release/plugins/cryptography/botan/libbotan.so" libbotan.so

# Copy test data
echo "Copying test data archive..."
cd "${project_dir}/build/linux/gcc/x86_64/release/test/"
cp "${project_dir}/src/tests/data/test-data.zip" test-data.zip

echo "Extracting test data..."
7z e test-data.zip -aos &> /dev/null

# Run tests
echo "Running tests..."
chmod +x tests
./tests

# Copy plugins for app
# echo "Copy plugins to app..."
# mkdir -p "${project_dir}/build/linux/gcc/x86_64/release/widgets/plugins/cryptography/botan/"
# cd "${project_dir}/build/linux/gcc/x86_64/release/widgets/plugins/cryptography/botan/"
# cp "${project_dir}/build/linux/gcc/x86_64/release/plugins/cryptography/botan/libbotan.so" libbotan.so

# Package Kryvo
echo "Packaging..."

echo "Copying app to packaging directory..."
cp -r "${project_dir}/build/linux/gcc/x86_64/release/widgets/." "${project_dir}/build/linux/gcc/x86_64/release/Kryvo/"

cd "${project_dir}/build/linux/gcc/x86_64/release/Kryvo/"

rm -rf moc
rm -rf obj
rm -rf qrc

echo "Copying Qt dependencies..."

cp -P "${qt_path}/lib/libicui18n.so" "libicui18n.so"
cp -P "${qt_path}/lib/libicui18n.so.56" "libicui18n.so.56"
cp "${qt_path}/lib/libicui18n.so.56.1" "libicui18n.so.56.1"
cp -P "${qt_path}/lib/libicuuc.so" "libicuuc.so"
cp -P "${qt_path}/lib/libicuuc.so.56" "libicuuc.so.56"
cp "${qt_path}/lib/libicuuc.so.56.1" "libicuuc.so.56.1"
cp -P "${qt_path}/lib/libicudata.so" "libicudata.so"
cp -P "${qt_path}/lib/libicudata.so.56" "libicudata.so.56"
cp "${qt_path}/lib/libicudata.so.56.1" "libicudata.so.56.1"

mkdir platforms
cp "${qt_path}/plugins/platforms/libqxcb.so" "platforms/libqxcb.so"
cp "${qt_path}/plugins/platforms/libqminimal.so" "platforms/libqminimal.so"

cp -P "${qt_path}/lib/libQt6Core.so" "libQt6Core.so"
cp -P "${qt_path}/lib/libQt6Core.so.6" "libQt6Core.so.6"
cp "${qt_path}/lib/libQt6Core.so.6.4.2" "libQt6Core.so.6.4.2"
cp -P "${qt_path}/lib/libQt6Gui.so" "libQt6Gui.so"
cp -P "${qt_path}/lib/libQt6Gui.so.6" "libQt6Gui.so.6"
cp "${qt_path}/lib/libQt6Gui.so.6.4.2" "libQt6Gui.so.6.4.2"
cp -P "${qt_path}/lib/libQt6OpenGL.so" "libQt6OpenGL.so"
cp -P "${qt_path}/lib/libQt6OpenGL.so.6" "libQt6OpenGL.so.6"
cp "${qt_path}/lib/libQt6OpenGL.so.6.4.2" "libQt6OpenGL.so.6.4.2"
cp -P "${qt_path}/lib/libQt6Svg.so" "libQt6Svg.so"
cp -P "${qt_path}/lib/libQt6Svg.so.6" "libQt6Svg.so.6"
cp "${qt_path}/lib/libQt6Svg.so.6.4.2" "libQt6Svg.so.6.4.2"
cp -P "${qt_path}/lib/libQt6DBus.so" "libQt6DBus.so"
cp -P "${qt_path}/lib/libQt6DBus.so.6" "libQt6DBus.so.6"
cp "${qt_path}/lib/libQt6DBus.so.6.4.2" "libQt6DBus.so.6.4.2"
cp -P "${qt_path}/lib/libQt6XcbQpa.so" "libQt6XcbQpa.so"
cp -P "${qt_path}/lib/libQt6XcbQpa.so.6" "libQt6XcbQpa.so.6"
cp "${qt_path}/lib/libQt6XcbQpa.so.6.4.2" "libQt6XcbQpa.so.6.4.2"
cp -P "${qt_path}/lib/libQt6Widgets.so" "libQt6Widgets.so"
cp -P "${qt_path}/lib/libQt6Widgets.so.6" "libQt6Widgets.so.6"
cp "${qt_path}/lib/libQt6Widgets.so.6.4.2" "libQt6Widgets.so.6.4.2"

chrpath -r \$ORIGIN/.. platforms/libqxcb.so
chrpath -r \$ORIGIN/.. platforms/libqminimal.so

cp "${project_dir}/Release Notes" "Release Notes"
cp "${project_dir}/README.md" "README.md"
cp "${project_dir}/LICENSE" "LICENSE"
cp "${project_dir}/Botan License" "Botan License"
cp "${project_dir}/Qt License" "Qt License"
mkdir themes
cp "${project_dir}/resources/stylesheets/kryvo.qss" "themes/kryvo.qss"

echo "Copying files for installer..."
mkdir -p "${project_dir}/installer/linux/packages/app.kryvo/data/"
cp -R * "${project_dir}/installer/linux/packages/app.kryvo/data/"

echo "Packaging portable archive..."
cd ..
7z a kryvo_${tag_name}_linux_x86_64_portable.zip Kryvo

echo "Creating installer..."
cd "${project_dir}/installer/linux/"
binarycreator --offline-only -c config/config.xml -p packages kryvo_${tag_name}_linux_x86_64_installer

echo "Done!"

exit 0
