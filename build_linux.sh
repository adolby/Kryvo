#!/bin/bash

set -o errexit -o nounset

# Hold on to current directory
project_dir=$(pwd)
qt_install_dir=/opt

# Get Qt
echo "Installing Qt..."
cd ${qt_install_dir}
echo "Downloading Qt files..."
wget https://github.com/adolby/qt-more-builds/releases/download/5.11.2/qt-opensource-5.11.2-linux-x86_64.7z
echo "Extracting Qt files..."
7z x qt-opensource-5.11.2-linux-x86_64.7z &> /dev/null

# Install Qt Installer Framework
echo "Installing Qt Installer Framework..."
wget https://github.com/adolby/qt-more-builds/releases/download/qt-ifw-3.0.4/qt-installer-framework-opensource-3.0.4-linux.7z
7z x qt-installer-framework-opensource-3.0.4-linux.7z &> /dev/null

# Add Qt binaries to path
echo "Adding Qt binaries to path..."
PATH=${qt_install_dir}/Qt/5.11.2/gcc_64/bin/:${qt_install_dir}/Qt/QtIFW3.0.4/bin/:${PATH}

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

cd ${project_dir}

# Clean build directory
make distclean
rm -rf ${project_dir}/build/linux/

# Build Kryvo
echo "Building Kryvo..."
qmake CONFIG+=release -spec linux-g++-64
make

# Copy Qt dependencies for test app
echo "Copy Qt dependencies to test app..."
cd ${project_dir}/build/linux/gcc/x86_64/release/test/
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libicui18n.so.56.1" "libicui18n.so.56"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libicuuc.so.56.1" "libicuuc.so.56"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libicudata.so.56.1" "libicudata.so.56"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libQt5Core.so.5.11.2" "libQt5Core.so.5"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libQt5Test.so.5.11.2" "libQt5Test.so.5"

# Copy plugins for test app
echo "Copy plugins for test app..."
mkdir -p ${project_dir}/build/linux/gcc/x86_64/release/test/plugins/cryptography/botan/
cd ${project_dir}/build/linux/gcc/x86_64/release/test/plugins/cryptography/botan/
cp ${project_dir}/build/linux/gcc/x86_64/release/plugins/cryptography/botan/libbotan.so libbotan.so

# Copy test data
echo "Copying test data archive..."
cd ${project_dir}/build/linux/gcc/x86_64/release/test/
cp ${project_dir}/src/tests/data/test-data.zip test-data.zip

echo "Extracting test data..."
7z x test-data.zip &> /dev/null

# Run tests
echo "Running tests..."
chmod +x CryptoTests
./CryptoTests

# Package Kryvo
echo "Packaging..."

# Copy plugins for app
mkdir -p ${project_dir}/build/linux/gcc/x86_64/release/Kryvo/plugins/cryptography/botan/
cd ${project_dir}/build/linux/gcc/x86_64/release/Kryvo/plugins/cryptography/botan/
cp ${project_dir}/build/linux/gcc/x86_64/release/plugins/cryptography/botan/libbotan.so libbotan.so

cd ${project_dir}/build/linux/gcc/x86_64/release/Kryvo/

rm -rf moc
rm -rf obj
rm -rf qrc

echo "Copying Qt dependencies..."

cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libicui18n.so.56.1" "libicui18n.so.56"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libicuuc.so.56.1" "libicuuc.so.56"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libicudata.so.56.1" "libicudata.so.56"

mkdir platforms
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/plugins/platforms/libqxcb.so" "platforms/libqxcb.so"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/plugins/platforms/libqminimal.so" "platforms/libqminimal.so"

cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libQt5Core.so.5.11.2" "libQt5Core.so.5"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libQt5Gui.so.5.11.2" "libQt5Gui.so.5"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libQt5Svg.so.5.11.2" "libQt5Svg.so.5"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libQt5DBus.so.5.11.2" "libQt5DBus.so.5"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libQt5XcbQpa.so.5.11.2" "libQt5XcbQpa.so.5"
cp "${qt_install_dir}/Qt/5.11.2/gcc_64/lib/libQt5Widgets.so.5.11.2" "libQt5Widgets.so.5"

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
cp -R * "${project_dir}/installer/linux/packages/io.kryvo/data/"

TAG_NAME="${TAG_NAME:-dev}"

echo "Packaging portable archive..."
cd ..
7z a kryvo_${TAG_NAME}_linux_x86_64_portable.zip Kryvo

echo "Creating installer..."
cd "${project_dir}/installer/linux/"
binarycreator --offline-only -c config/config.xml -p packages kryvo_${TAG_NAME}_linux_x86_64_installer

echo "Done!"

exit 0
