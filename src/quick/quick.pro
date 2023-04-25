include(../../defaults.pri)

QT += core gui qml quick quickcontrols2

TARGET = Kryvo

TEMPLATE = app

CONFIG += c++17

# Qt Creator Debug/Release Differentiation
# Ensure one "debug_and_release" in CONFIG, for clarity.
debug_and_release {
  CONFIG -= debug_and_release
  CONFIG += debug_and_release
}
# Ensure one "debug" or "release" in CONFIG so they can be used as conditionals
# instead of writing "CONFIG(debug, debug|release)"
CONFIG(debug, debug|release) {
  CONFIG -= debug release
  CONFIG += debug
}
CONFIG(release, debug|release) {
  CONFIG -= debug release
  CONFIG += release
}

SOURCES += \
  main.cpp \
  Application.cpp \
  Ui.cpp

HEADERS += \
  Application.hpp \
  Ui.hpp

# Platform-specific configuration
linux {
  message(Linux)

  android {
    message(Android)

    QT += androidextras

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../../resources/android/package

    DISTFILES += \
      ../../resources/android/package/AndroidManifest.xml \
      ../../resources/android/package/build.gradle \
      ../../resources/android/package/gradle/wrapper/gradle-wrapper.jar \
      ../../resources/android/package/gradle/wrapper/gradle-wrapper.properties \
      ../../resources/android/package/gradlew \
      ../../resources/android/package/gradlew.bat \
      ../../resources/android/package/res/values/libs.xml \
      ../../resources/android/package/res/drawable-mdpi/icon.png \
      ../../resources/android/package/res/drawable-hdpi/icon.png \
      ../../resources/android/package/res/drawable-xhdpi/icon.png \
      ../../resources/android/package/res/drawable-xxhdpi/icon.png \
      ../../resources/android/package/res/drawable-xxxhdpi/icon.png \
      ../../resources/android/package/res/drawable/splash.xml \
      ../../resources/android/package/res/drawable/launch_icon.png

    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
      message(armeabi-v7a)

      debug {
        message(Debug)
        LIBS += -L$$PWD/../../build/android/armv7/debug/core -lcore
        LIBS += -L$$PWD/../../build/android/armv7/debug/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/armv7/debug/plugins/cryptography/botan -lbotan
        LIBS += -L$$PWD/../../build/android/armv7/debug/plugins/cryptography/openssl -lopenssl
        DESTDIR = $$PWD/../../build/android/armv7/debug/quick
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/android/armv7/release/core -lcore
        LIBS += -L$$PWD/../../build/android/armv7/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/armv7/release/plugins/cryptography/botan -lbotan
        LIBS += -L$$PWD/../../build/android/armv7/release/plugins/cryptography/openssl -lopenssl
        DESTDIR = $$PWD/../../build/android/armv7/release/quick
      }
    }

    contains(ANDROID_TARGET_ARCH, arm64-v8a) {
      message(arm64-v8a)

      debug {
        message(Debug)
        LIBS += -L$$PWD/../../build/android/arm64_v8a/debug/core -lcore
        LIBS += -L$$PWD/../../build/android/arm64_v8a/debug/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/arm64_v8a/debug/plugins/cryptography/botan -lbotan
        LIBS += -L$$PWD/../../build/android/arm64_v8a/debug/plugins/cryptography/openssl -lopenssl

        DESTDIR = $$PWD/../../build/android/arm64_v8a/debug/quick
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/android/arm64_v8a/release/core -lcore
        LIBS += -L$$PWD/../../build/android/arm64_v8a/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/arm64_v8a/release/plugins/cryptography/botan -lbotan
        LIBS += -L$$PWD/../../build/android/arm64_v8a/release/plugins/cryptography/openssl -lopenssl
        DESTDIR = $$PWD/../../build/android/arm64_v8a/release/quick
      }
    }
  } # End android

  linux-clang {
    message(clang)

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/release/quick
    }
  } # End linux-clang

  linux-g++ {
    message(g++)

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/release/quick
    }
  } # End linux-g++
} # End linux

darwin {
  ios {
    message(iOS)
    message(clang)

    CONFIG -= simulator

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/iOS/debug/core -lcore
      LIBS += -L$$PWD/../../build/iOS/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/iOS/debug/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/iOS/debug/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/iOS/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/iOS/release/core -lcore
      LIBS += -L$$PWD/../../build/iOS/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/iOS/release/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/iOS/release/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/iOS/release/quick
    }

    LIBS += -framework Foundation -framework CoreFoundation -framework UIKit

    QMAKE_INFO_PLIST = $$PWD/../../resources/ios/plist/Info.plist

    ios_icon.files = $$files($$PWD/../../resources/ios/icon/AppIcon*.png)
    QMAKE_BUNDLE_DATA += ios_icon

    launch.files = $$PWD/../../resources/ios/launch/Launch.xib $$PWD/../../resources/ios/launch/LaunchIcon.png
    QMAKE_BUNDLE_DATA += launch

    QMAKE_FULL_VERSION = 1.0.0
    QMAKE_SHORT_VERSION = 1.0
  } # End ios

  macos {
    message(macOS)
    message(clang)

    INCLUDEPATH += /usr/local/opt/openssl@3/include

    QMAKE_TARGET_BUNDLE_PREFIX = app.kryvo
    ICON = $$PWD/../../resources/icons/kryvo.icns

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/release/quick
    }
  } # End macos

  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  # Install OpenSSL v3 from Homebrew
  INCLUDEPATH += /usr/local/opt/openssl@3/include
  LIBS += -L/usr/local/opt/openssl@3/lib -lssl -lcrypto
} # End darwin

win32 {
  message(Windows)

  win32-msvc {
    message(MSVC)

    LIBS += -ladvapi32 -luser32 -lws2_32

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(x86_64)

      debug {
        message(Debug)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/core -lcore
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/lib/zlib -lz
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/plugins/cryptography/botan -lbotan
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/plugins/cryptography/openssl -lopenssl
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/debug/quick
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/core -lcore
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/plugins/cryptography/botan -lbotan
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/plugins/cryptography/openssl -lopenssl
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/release/quick
      }

      LIBS += -L$${OPENSSL_PATH}/OpenSSLv3/Win_x64/lib -lssl -lcrypto
    }
  } # End win32-msvc

  RC_ICONS += $$PWD/../../resources/icons/kryvo.ico
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc

RESOURCES += $$PWD/../../resources/images.qrc $$PWD/quick.qrc
