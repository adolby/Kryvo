#!/bin/bash

set -o errexit -o nounset

project_dir=$(pwd)
qt_install_dir=${HOME}
ndk_install_dir=${HOME}

# Get Qt
echo "Installing Qt..."
cd "${qt_install_dir}"
echo "Downloading Qt files..."
wget --timestamping --quiet https://github.com/adolby/qt-more-builds/releases/download/5.12.4/qt-opensource-5.12.4-android-armv7.zip &> /dev/null
echo "Extracting Qt files..."
unzip -qq qt-opensource-5.12.4-android-armv7.zip &> /dev/null

# Add Qt binaries to path
echo "Adding Qt binaries to path..."
PATH="${qt_install_dir}/Qt/5.12.4/android_armv7/bin/:${PATH}"

# Check qmake version
qmake --version

cd "${ndk_install_dir}"
wget --timestamping --quiet https://dl.google.com/android/repository/android-ndk-r19c-linux-x86_64.zip &> /dev/null
unzip -qq android-ndk-r19c-linux-x86_64.zip &> /dev/null

export ANDROID_NDK_ROOT=${ndk_install_dir}/android-ndk-r19c
export ANDROID_SDK_ROOT=/usr/local/android-sdk
export ANDROID_NDK_PLATFORM=android-21
PATH=${ANDROID_NDK_ROOT}:${PATH}

# Get Botan
# echo "Installing Botan..."
# wget https://github.com/randombit/botan/archive/1.11.32.zip
# unzip 1.11.32.zip &>/dev/null
# chmod -R +x /usr/local/botan-1.11.32/
# cd /usr/local/botan-1.11.32/
# ./configure.py --cc=clang --amalgamation --disable-shared --with-zlib
# cp botan_all_aesni.cpp ${project_dir}/src/cryptography/botan/android/armv7/botan_all_aesni.cpp
# cp botan_all_avx2.cpp ${project_dir}/src/cryptography/botan/android/armv7/botan_all_avx2.cpp
# cp botan_all_internal.h ${project_dir}/src/cryptography/botan/android/armv7/botan_all_internal.h
# cp botan_all_rdrand.cpp ${project_dir}/src/cryptography/botan/android/armv7/botan_all_rdrand.cpp
# cp botan_all_rdseed.cpp ${project_dir}/src/cryptography/botan/android/armv7/botan_all_rdseed.cpp
# cp botan_all_ssse3.cpp ${project_dir}/src/cryptography/botan/android/armv7/botan_all_ssse3.cpp
# cp botan_all.cpp ${project_dir}/src/cryptography/botan/android/armv7/botan_all.cpp
# cp botan_all.h ${project_dir}/src/cryptography/botan/android/armv7/botan_all.h

cd "${project_dir}"

# Clean build directory
rm -rf "${project_dir}/build/android/armv7/"

mkdir -p "${project_dir}/build/android/armv7/release/widgets/"
mkdir -p "${project_dir}/build/android/armv7/release/widgets/moc"
mkdir -p "${project_dir}/build/android/armv7/release/widgets/qrc"
mkdir -p "${project_dir}/build/android/armv7/release/widgets/obj"
mkdir -p "${project_dir}/build/android/armv7/release/quick/"
mkdir -p "${project_dir}/build/android/armv7/release/quick/moc"
mkdir -p "${project_dir}/build/android/armv7/release/quick/qrc"
mkdir -p "${project_dir}/build/android/armv7/release/quick/obj"
mkdir -p "${project_dir}/build/android/armv7/release/lib/"
mkdir -p "${project_dir}/build/android/armv7/release/lib/zlib/"
mkdir -p "${project_dir}/build/android/armv7/release/test/"

# Build Kryvo
echo "Building Kryvo..."

if [ -f "${project_dir}/Makefile" ]; then
  make distclean
fi

qmake -spec android-clang CONFIG+=release
make

# Copy plugins for test app
# echo "Copying plugins for test app..."
# mkdir -p "${project_dir}/build/android/armv7/release/test/plugins/cryptography/botan/"
# cd "${project_dir}/build/android/armv7/release/test/plugins/cryptography/botan/"
# cp "${project_dir}/build/android/armv7/release/plugins/cryptography/botan/libbotan.so" libbotan.so

# Copy test data
# echo "Copying test data archive..."
# cd "${project_dir}/build/android/armv7/release/test/"
# cp "${project_dir}/src/tests/data/test-data.zip" test-data.zip

# echo "Extracting test data..."
# unzip test-data.zip -aos &> /dev/null

# Run tests
# echo "Running tests..."
# chmod +x tests
# ./tests

# Package Kryvo
echo "Packaging..."

make install INSTALL_ROOT="${project_dir}/build/android/armv7/release/android-build/"

echo "Copying app dependencies..."
androiddeployqt --input "${project_dir}/src/quick/android-libKryvo.so-deployment-settings.json" --output "${project_dir}/build/android/armv7/release/android-build" --gradle --release --sign "${project_dir}/resources/android/android_release.keystore" ${keystore_alias} --storepass ${keystore_password}

TAG_NAME="${TAG_NAME:-dev}"

mv "${project_dir}/build/android/armv7/release/android-build/build/outputs/apk/release/android-build-release-signed.apk" "${project_dir}/build/android/armv7/release/kryvo_${TAG_NAME}_android_armv7.apk"

echo "Done!"

exit 0
