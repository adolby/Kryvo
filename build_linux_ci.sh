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

cp -a "${qt_path}/lib/libicui18n.so.56.1" "libicui18n.so.56.1"
ln -s "libicui18n.so.56.1" "libicui18n.so"
ln -s "libicui18n.so.56.1" "libicui18n.so.56"

cp -a "${qt_path}/lib/libicuuc.so.56.1" "libicuuc.so.56.1"
ln -s "libicuuc.so.56.1" "libicuuc.so"
ln -s "libicuuc.so.56.1" "libicuuc.so.56"

cp -a "${qt_path}/lib/libicudata.so.56.1" "libicudata.so.56.1"
ln -s "libicudata.so.56.1" "libicudata.so"
ln -s "libicudata.so.56.1" "libicudata.so.56"

cp -a "${qt_path}/lib/libQt6Core.so.6.4.2" "libQt6Core.so.6.4.2"
ln -s "libQt6Core.so.6.4.2" "libQtCore.so"
ln -s "libQt6Core.so.6.4.2" "libQtCore.so.6"

cp -a "${qt_path}/lib/libQt6Test.so.6.4.2" "libQt6Test.so.6.4.2"
ln -s "libQt6Test.so.6.4.2" "libQtTest.so"
ln -s "libQt6Test.so.6.4.2" "libQtTest.so.6"

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
cp -a "${project_dir}/build/linux/gcc/x86_64/release/widgets/." "${project_dir}/build/linux/gcc/x86_64/release/Kryvo/"

cd "${project_dir}/build/linux/gcc/x86_64/release/Kryvo/"

rm -rf moc
rm -rf obj
rm -rf qrc

echo "Copying Qt dependencies..."

cp -a "${qt_path}/lib/libicui18n.so.56.1" "libicui18n.so.56.1"
ln -s "libicui18n.so.56.1" "libicui18n.so"
ln -s "libicui18n.so.56.1" "libicui18n.so.56"

cp -a "${qt_path}/lib/libicuuc.so.56.1" "libicuuc.so.56.1"
ln -s "libicuuc.so.56.1" "libicuuc.so"
ln -s "libicuuc.so.56.1" "libicuuc.so.56"

cp -a "${qt_path}/lib/libicudata.so.56.1" "libicudata.so.56.1"
ln -s "libicudata.so.56.1" "libicudata.so"
ln -s "libicudata.so.56.1" "libicudata.so.56"

mkdir platforms
cp -a "${qt_path}/plugins/platforms/libqxcb.so" "platforms/libqxcb.so"
cp -a "${qt_path}/plugins/platforms/libqminimal.so" "platforms/libqminimal.so"

cp -a "${qt_path}/lib/libQt6Core.so.6.4.2" "libQt6Core.so.6.4.2"
ln -s "libQt6Core.so.6.4.2" "libQt6Core.so"
ln -s "libQt6Core.so.6.4.2" "libQt6Core.so.6"

cp -a "${qt_path}/lib/libQt6Gui.so.6.4.2" "libQt6Gui.so.6.4.2"
ln -s "libQt6Gui.so.6.4.2" "libQt6Gui.so"
ln -s "libQt6Gui.so.6.4.2" "libQt6Gui.so.6"

cp -a "${qt_path}/lib/libQt6OpenGL.so.6.4.2" "libQt6OpenGL.so.6.4.2"
ln -s "libQt6OpenGL.so.6.4.2" "libQt6OpenGL.so"
ln -s "libQt6OpenGL.so.6.4.2" "libQt6OpenGL.so.6"

cp -a "${qt_path}/lib/libQt6Svg.so.6.4.2" "libQt6Svg.so.6.4.2"
ln -s "libQt6Svg.so.6.4.2" "libQt6Svg.so"
ln -s "libQt6Svg.so.6.4.2" "libQt6Svg.so.6"

cp -a "${qt_path}/lib/libQt6DBus.so.6.4.2" "libQt6DBus.so.6.4.2"
ln -s "libQt6DBus.so.6.4.2" "libQt6DBus.so"
ln -s "libQt6DBus.so.6.4.2" "libQt6DBus.so.6"

cp -a "${qt_path}/lib/libQt6XcbQpa.so.6.4.2" "libQt6XcbQpa.so.6.4.2"
ln -s "libQt6XcbQpa.so.6.4.2" "libQt6XcbQpa.so"
ln -s "libQt6XcbQpa.so.6.4.2" "libQt6XcbQpa.so.6"

cp -a "${qt_path}/lib/libQt6Widgets.so.6.4.2" "libQt6Widgets.so.6.4.2"
ln -s "libQt6Widgets.so.6.4.2" "libQt6Widgets.so"
ln -s "libQt6Widgets.so.6.4.2" "libQt6Widgets.so.6"

chrpath -r \$ORIGIN/.. platforms/libqxcb.so
chrpath -r \$ORIGIN/.. platforms/libqminimal.so

cp -a "${project_dir}/Release Notes" "Release Notes"
cp -a "${project_dir}/README.md" "README.md"
cp -a "${project_dir}/LICENSE" "LICENSE"
cp -a "${project_dir}/Botan License" "Botan License"
cp -a "${project_dir}/Qt License" "Qt License"
mkdir themes
cp -a "${project_dir}/resources/stylesheets/kryvo.qss" "themes/kryvo.qss"

echo "Copying files for installer..."
mkdir -p "${project_dir}/installer/linux/packages/app.kryvo/data/"
cp -a * "${project_dir}/installer/linux/packages/app.kryvo/data/"

echo "Packaging portable archive..."
cd ..
tar czvf kryvo_${tag_name}_linux_x86_64_portable.tar.gz Kryvo

echo "Creating installer..."
cd "${project_dir}/installer/linux/"
binarycreator --offline-only -c config/config.xml -p packages kryvo_${tag_name}_linux_x86_64_installer

echo "Done!"

exit 0
