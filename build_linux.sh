#!/bin/bash

set -o errexit -o nounset

# Update platform
sudo apt-get update
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-6 100
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

project=$(pwd)

# Get Qt
cd /usr/local/
sudo wget https://github.com/adolby/qt-more-builds/releases/download/5.7/qt-opensource-5.7.0-x86_64-gcc6.zip
sudo 7z x qt-opensource-5.7.0-x86_64-gcc6.zip &>/dev/null
sudo chmod -R +x /usr/local/Qt-5.7.0/bin/

# Install Qt Installer Framework
sudo wget https://github.com/adolby/qt-more-builds/releases/download/qt-ifw-2.0.3/qt-installer-framework-opensource-2.0.3.zip
sudo 7z x qt-installer-framework-opensource-2.0.3.zip &>/dev/null
sudo chmod -R +x /usr/local/QtIFW2.0.3/bin/

# Build
echo ${project}
cd ${project}/src/
/usr/local/Qt-5.7.0/bin/qmake -config release
make -j2

# Run tests
cd tests
/usr/local/Qt-5.7.0/bin/qmake -config release
make -j2
cd ../../build/linux/gcc/x86_64/release/test/
sudo chmod +x CryptoTests
./CryptoTests

# Package
cd ..
cp "/usr/local/Qt-5.7.0/lib/libQt5Core.so.5.7.0" "Kryvos/libQt5Core.so"
cp "/usr/local/Qt-5.7.0/lib/libQt5Gui.so.5.7.0" "Kryvos/libQt5Gui.so"
cp "/usr/local/Qt-5.7.0/lib/libQt5Svg.so.5.7.0" "Kryvos/libQt5Svg.so"
cp "/usr/local/Qt-5.7.0/lib/libQt5Widgets.so.5.7.0" "Kryvos/libQt5Widgets.so"
#mkdir platforms
#cp "/usr/local/Qt-5.7.0/lib/libQt5XcbQpa.so.5.7.0" "platforms/libQt5XcbQpa.so"
#cp "/usr/local/Qt-5.7.0/lib/libQt5DBus.so.5.7.0" "platforms/libQt5DBus.so"
#cp "../../../../../installer/linux/packages/com.kryvosproject.kryvos/data/Kryvos.sh"
cp "../../../../../Release Notes" "Kryvos/Release Notes"
cp "../../../../../README.md" "Kryvos/README.md"
cp "../../../../../LICENSE" "Kryvos/LICENSE"
cp "../../../../../Botan License" "Kryvos/Botan License"
cp "../../../../../Qt License" "Kryvos/Qt License"
cp -R Kryvos/ "../../../../../installer/linux/packages/com.kryvosproject.kryvos/data/"
7z a kryvos_${TRAVIS_TAG}_linux_x86_64_portable.zip "Kryvos" "libQt5Core.so" "libQt5Gui.so" "libQt5Svg.so" "libQt5Widgets.so" "Release Notes" "README.md" "LICENSE" "Botan License" "Qt License"
cd "../../../../../installer/linux/"
/usr/local/QtIFW2.0.3/bin/binarycreator --offline-only -c config\config.xml -p packages kryvos_${TRAVIS_TAG}_linux_x86_64_installer

exit 0
