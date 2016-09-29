#!/bin/bash

set -o errexit -o nounset

# Update platform
echo "Updating platform..."
sudo apt-get update
gcc --version
g++ --version
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" upgrade
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install p7zip-full
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libfontconfig1
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libfontconfig1-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libfreetype6-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libx11-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxext-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxfixes-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxi-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxrender-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxcb1-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libx11-xcb-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libxcb-glx0-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install mesa-common-dev
sudo apt-get -qq -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" install libglu1-mesa-dev

project_dir=$(pwd)

# Get Qt
echo "Installing Qt..."
cd /usr/local/
sudo wget https://github.com/adolby/qt-more-builds/releases/download/5.7/qt-opensource-5.7.0-x86_64-linux-gcc6.zip
sudo 7z x qt-opensource-5.7.0-x86_64-linux-gcc6.zip &>/dev/null
sudo chmod -R +x /usr/local/Qt-5.7.0/bin/

# Install Qt Installer Framework
echo "Installing Qt Installer Framework..."
sudo wget https://github.com/adolby/qt-more-builds/releases/download/qt-ifw-2.0.3/qt-installer-framework-opensource-2.0.3.zip
sudo 7z x qt-installer-framework-opensource-2.0.3.zip &>/dev/null
sudo chmod -R +x /usr/local/QtIFW2.0.3/bin/

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
echo ${project_dir}
cd ${project_dir}/src/
/usr/local/Qt-5.7.0/bin/qmake -config release
make -j2

# Build tests
echo "Building tests..."
cd ${project_dir}/src/tests/
/usr/local/Qt-5.7.0/bin/qmake -config release
make -j2

# Run tests
echo "Running tests..."
cd ${project_dir}/build/linux/gcc/x86_64/release/test/
sudo chmod +x CryptoTests
./CryptoTests

# Package Kryvos
echo "Packaging..."
cd ${project_dir}/build/linux/gcc/x86_64/release/Kryvos/
rm -rf moc
rm -rf obj
rm -rf qrc
cp "/usr/local/Qt-5.7.0/lib/libQt5Core.so.5.7.0" "libQt5Core.so"
cp "/usr/local/Qt-5.7.0/lib/libQt5Gui.so.5.7.0" "libQt5Gui.so"
cp "/usr/local/Qt-5.7.0/lib/libQt5Svg.so.5.7.0" "libQt5Svg.so"
cp "/usr/local/Qt-5.7.0/lib/libQt5Widgets.so.5.7.0" "libQt5Widgets.so"
cp "${project_dir}/Release Notes" "Release Notes"
cp "${project_dir}/README.md" "README.md"
cp "${project_dir}/LICENSE" "LICENSE"
cp "${project_dir}/Botan License" "Botan License"
cp "${project_dir}/Qt License" "Qt License"
mkdir themes
cp "${project_dir}/src/resources/stylesheets/kryvos.qss" "themes/kryvos.qss"

echo "Packaging portable archive..."
cp -R * "${project_dir}/installer/linux/packages/com.kryvosproject.kryvos/data/"
cd ..
7z a kryvos_${TAG_NAME}_linux_x86_64_portable.zip Kryvos

echo "Building installer..."
cd "${project_dir}/installer/linux/"
/usr/local/QtIFW2.0.3/bin/binarycreator --offline-only -c config/config.xml -p packages kryvos_${TAG_NAME}_linux_x86_64_installer

echo "Done!"

exit 0
