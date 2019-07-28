#!/bin/bash

set -o errexit -o nounset

project_dir=$(pwd)
qt_install_dir=~
ndk_install_dir=~

# Get Qt
echo "Installing Qt..."
cd "${qt_install_dir}"
echo "Downloading Qt files..."
wget -N https://github.com/adolby/qt-more-builds/releases/download/5.12.4/qt-opensource-5.12.4-android-arm64_v8a.zip
echo "Extracting Qt files..."
unzip -qq qt-opensource-5.12.4-android-arm64_v8a.zip

# Add Qt binaries to path
echo "Adding Qt binaries to path..."
PATH="${qt_install_dir}/Qt/5.12.4/android_arm64_v8a/bin/:${PATH}"

# Check qmake version
qmake --version

cd "${ndk_install_dir}"
wget -N https://dl.google.com/android/repository/android-ndk-r19c-linux-x86_64.zip
unzip -qq android-ndk-r19c-linux-x86_64.zip

export ANDROID_NDK_ROOT=~/android-ndk-r19c
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
# cp botan_all_aesni.cpp ${project_dir}/src/cryptography/botan/android/arm64_v8a/botan_all_aesni.cpp
# cp botan_all_avx2.cpp ${project_dir}/src/cryptography/botan/android/arm64_v8a/botan_all_avx2.cpp
# cp botan_all_internal.h ${project_dir}/src/cryptography/botan/android/arm64_v8a/botan_all_internal.h
# cp botan_all_rdrand.cpp ${project_dir}/src/cryptography/botan/android/arm64_v8a/botan_all_rdrand.cpp
# cp botan_all_rdseed.cpp ${project_dir}/src/cryptography/botan/android/arm64_v8a/botan_all_rdseed.cpp
# cp botan_all_ssse3.cpp ${project_dir}/src/cryptography/botan/android/arm64_v8a/botan_all_ssse3.cpp
# cp botan_all.cpp ${project_dir}/src/cryptography/botan/android/arm64_v8a/botan_all.cpp
# cp botan_all.h ${project_dir}/src/cryptography/botan/android/arm64_v8a/botan_all.h

cd "${project_dir}"

# Clean build directory
rm -rf "${project_dir}/build/android/arm64_v8a/"

mkdir -p "${project_dir}/build/android/arm64_v8a/release/widgets/"
mkdir -p "${project_dir}/build/android/arm64_v8a/release/widgets/moc"
mkdir -p "${project_dir}/build/android/arm64_v8a/release/widgets/qrc"
mkdir -p "${project_dir}/build/android/arm64_v8a/release/widgets/obj"
mkdir -p "${project_dir}/build/android/arm64_v8a/release/quick/"
mkdir -p "${project_dir}/build/android/arm64_v8a/release/quick/moc"
mkdir -p "${project_dir}/build/android/arm64_v8a/release/quick/qrc"
mkdir -p "${project_dir}/build/android/arm64_v8a/release/quick/obj"
mkdir -p "${project_dir}/build/android/arm64_v8a/release/lib/"
mkdir -p "${project_dir}/build/android/arm64_v8a/release/lib/zlib/"
mkdir -p "${project_dir}/build/android/arm64_v8a/release/test/"
mkdir -p "${project_dir}/build/android/arm64_v8a/release/Kryvo/"

# Build Kryvo
echo "Building Kryvo..."

if [ -f "${project_dir}/Makefile" ]; then
  make distclean
fi

qmake CONFIG+=release -spec android-clang
make

# Copy plugins for test app
# echo "Copying plugins for test app..."
# mkdir -p "${project_dir}/build/android/arm64_v8a/release/test/plugins/cryptography/botan/"
# cd "${project_dir}/build/android/arm64_v8a/release/test/plugins/cryptography/botan/"
# cp "${project_dir}/build/android/arm64_v8a/release/plugins/cryptography/botan/libbotan.so" libbotan.so

# Copy test data
# echo "Copying test data archive..."
# cd "${project_dir}/build/android/arm64_v8a/release/test/"
# cp "${project_dir}/src/tests/data/test-data.zip" test-data.zip

# echo "Extracting test data..."
# unzip test-data.zip -aos &> /dev/null

# Run tests
# echo "Running tests..."
# chmod +x tests
# ./tests

# Copy plugins for app
# echo "Copy plugins to app..."
# mkdir -p "${project_dir}/build/android/arm64_v8a/release/widgets/plugins/cryptography/botan/"
# cd "${project_dir}/build/android/arm64_v8a/release/widgets/plugins/cryptography/botan/"
# cp "${project_dir}/build/android/arm64_v8a/release/plugins/cryptography/botan/libbotan.so" libbotan.so

# Package Kryvo
echo "Packaging..."

echo "Copying app dependencies..."
androiddeployqt --input "${project_dir}/build/android/arm64_v8a/release/Kryvo/android-libKryvo.so-deployment-settings.json" --output "${project_dir}/build/android/arm64_v8a/release/Kryvo/android-build" --android-platform android-28 --gradle

TAG_NAME="${TAG_NAME:-dev}"

mv "${project_dir}/build/android/arm64_v8a/release/Kryvo/android-build/build/outputs/apk/debug/android-build-debug.apk" "${project_dir}/build/android/arm64_v8a/release/Kryvo/kryvo_${TAG_NAME}_android_arm64_v8a.apk"

echo "Done!"

exit 0
