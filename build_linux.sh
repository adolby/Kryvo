#!/bin/bash

set -o errexit -o nounset

# Update platform
echo "Updating platform..."
sudo -E apt-get -yq --no-install-suggests --no-install-recommends --force-yes install p7zip-full
sudo -E apt-get -yq --no-install-suggests --no-install-recommends --force-yes install chrpath

# Hold on to current directory
project_dir=$(pwd)

# Get Qt
echo "Installing Qt..."
cd /usr/local/
echo "Downloading Qt files..."
sudo wget https://github.com/adolby/qt-more-builds/releases/download/5.7/qt-opensource-5.7.0-x86_64-linux-gcc6.zip
echo "Extracting Qt files..."
sudo 7z x qt-opensource-5.7.0-x86_64-linux-gcc6.zip &> /dev/null

# Install Qt Installer Framework
echo "Installing Qt Installer Framework..."
sudo wget https://github.com/adolby/qt-more-builds/releases/download/qt-ifw-2.0.3/qt-installer-framework-opensource-2.0.3.zip
sudo 7z x qt-installer-framework-opensource-2.0.3.zip &> /dev/null

# Add Qt binaries to path
echo "Adding Qt binaries to path..."
PATH=/usr/local/Qt-5.7.0/bin/:/usr/local/QtIFW2.0.3/bin/:${PATH}

# Get Botan
# echo "Installing Botan..."
# sudo wget https://github.com/randombit/botan/archive/1.11.32.zip
# sudo 7z x 1.11.32.zip &>/dev/null
# sudo chmod -R +x /usr/local/botan-1.11.32/
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

# Build Kryvos
echo "Building Kryvos..."
cd ${project_dir}
qmake -config release
make -j2

# Build tests
echo "Building tests..."
cd ${project_dir}/tests/
qmake -config release
make -j2

# Copy test dependencies
echo "Copying test dependencies..."
cp "/usr/local/Qt-5.7.0/lib/libQt5Core.so.5.7.0" "libQt5Core.so.5"
cp "/usr/local/Qt-5.7.0/lib/libQt5Test.so.5.7.0" "libQt5Test.so.5"

# Copy test data
echo "Copying test data..."
cd ${project_dir}/build/linux/gcc/x86_64/release/test/
cp ${project_dir}/tests/data/test-data.zip test-data.zip
7z x test-data.zip &> /dev/null

# Run tests
echo "Running tests..."
sudo chmod +x CryptoTests
./CryptoTests

# Package Kryvos
echo "Packaging..."
cd ${project_dir}/build/linux/gcc/x86_64/release/Kryvos/

rm -rf moc
rm -rf obj
rm -rf qrc

echo "Copying files for archival..."
mkdir platforms
cp "/usr/local/Qt-5.7.0/plugins/platforms/libqxcb.so" "platforms/libqxcb.so"
cp "/usr/local/Qt-5.7.0/plugins/platforms/libqminimal.so" "platforms/libqminimal.so"
cp "/usr/local/Qt-5.7.0/lib/libQt5Core.so.5.7.0" "libQt5Core.so.5"
cp "/usr/local/Qt-5.7.0/lib/libQt5Gui.so.5.7.0" "libQt5Gui.so.5"
cp "/usr/local/Qt-5.7.0/lib/libQt5Svg.so.5.7.0" "libQt5Svg.so.5"
cp "/usr/local/Qt-5.7.0/lib/libQt5DBus.so.5.7.0" "libQt5DBus.so.5"
cp "/usr/local/Qt-5.7.0/lib/libQt5XcbQpa.so.5.7.0" "libQt5XcbQpa.so.5"
cp "/usr/local/Qt-5.7.0/lib/libQt5Widgets.so.5.7.0" "libQt5Widgets.so.5"

chrpath -r \$ORIGIN/.. platforms/libqxcb.so
chrpath -r \$ORIGIN/.. platforms/libqminimal.so

cp "${project_dir}/Release Notes" "Release Notes"
cp "${project_dir}/README.md" "README.md"
cp "${project_dir}/LICENSE" "LICENSE"
cp "${project_dir}/Botan License" "Botan License"
cp "${project_dir}/Qt License" "Qt License"
mkdir themes
cp "${project_dir}/resources/stylesheets/kryvos.qss" "themes/kryvos.qss"

echo "Copying files for installer..."
cp -R * "${project_dir}/installer/linux/packages/com.kryvosproject.kryvos/data/"

echo "Packaging portable archive..."
cd ..
7z a kryvos_${TAG_NAME}_linux_x86_64_portable.zip Kryvos

echo "Creating installer..."
cd "${project_dir}/installer/linux/"
binarycreator --offline-only -c config/config.xml -p packages kryvos_${TAG_NAME}_linux_x86_64_installer

echo "Done!"

exit 0
