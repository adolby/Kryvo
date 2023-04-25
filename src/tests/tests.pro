include(../../defaults.pri)

QT += testlib
QT -= gui

TARGET = tests

TEMPLATE = app

CONFIG += console warn_on testcase c++17

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
  test_Crypto.cpp \
  test_Archiver.cpp \
  FileOperations.cpp \
  test_FileOperations.cpp \
  main.cpp

HEADERS += \
  FileOperations.hpp

# Platform-specific configuration
linux {
  message(Linux)

  android {
    message(Android)

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../resources/android

    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
      message(armeabi-v7a)

      debug {
        message(Debug)
        LIBS += -L$$PWD/../../build/android/armv7/debug/core -lcore
        LIBS += -L$$PWD/../../build/android/armv7/debug/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/armv7/debug/plugins/cryptography/botan -lbotan
        LIBS += -L$$PWD/../../build/android/armv7/debug/plugins/cryptography/openssl -lopenssl
        DESTDIR = $$PWD/../../build/android/armv7/debug/test
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/android/armv7/release/core -lcore
        LIBS += -L$$PWD/../../build/android/armv7/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/armv7/release/plugins/cryptography/botan -lbotan
        LIBS += -L$$PWD/../../build/android/armv7/release/plugins/cryptography/openssl -lopenssl
        DESTDIR = $$PWD/../../build/android/armv7/release/test
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
        DESTDIR = $$PWD/../../build/android/arm64_v8a/debug/test
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/android/arm64_v8a/release/core -lcore
        LIBS += -L$$PWD/../../build/android/arm64_v8a/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/arm64_v8a/release/plugins/cryptography/botan -lbotan
        LIBS += -L$$PWD/../../build/android/arm64_v8a/release/plugins/cryptography/openssl -lopenssl
        DESTDIR = $$PWD/../../build/android/arm64_v8a/release/test
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
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/debug/test
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/release/test
    }

    LIBS += -L$${OPENSSL_PATH}/lib -lssl -lcrypto
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
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/debug/test
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/release/test
    }

    LIBS += -L$${OPENSSL_PATH}/lib -lssl -lcrypto
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
      DESTDIR = $$PWD/../../build/iOS/debug/test
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/iOS/release/core -lcore
      LIBS += -L$$PWD/../../build/iOS/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/iOS/release/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/iOS/release/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/iOS/release/test
    }
  } # End ios

  macos {
    message(macOS)
    message(clang)

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/debug/test
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/plugins/cryptography/botan -lbotan
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/plugins/cryptography/openssl -lopenssl
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/release/test
    }
  } # End macos

  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  # Install OpenSSL v3 from Homebrew
  INCLUDEPATH += $${OPENSSL_PATH}/include
  LIBS += -L$${OPENSSL_PATH}/lib -lssl -lcrypto
} # End darwin

win32 {
  message(Windows)

  win32-msvc {
    LIBS += -ladvapi32 -luser32 -lws2_32

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(MSVC x86_64)

      debug {
        message(Debug)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/core -lcore
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/lib/zlib -lz
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/plugins/cryptography/botan -lbotan
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/plugins/cryptography/openssl -lopenssl
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/debug/test
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/core -lcore
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/plugins/cryptography/botan -lbotan
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/plugins/cryptography/openssl -lopenssl
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/release/test
      }

      LIBS += -L$${OPENSSL_PATH}/lib -llibssl -llibcrypto
    }
  } # End win32-msvc
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc
