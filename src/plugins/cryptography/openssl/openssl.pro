include(../../../../defaults.pri)

QT += core

TARGET = openssl

TEMPLATE = lib

CONFIG += plugin static c++17

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

SOURCES += OpenSslProvider.cpp

HEADERS += \
  OpenSslProvider.hpp \
  $$PWD/../../../core/Constants.hpp \
  $$PWD/../../../core/DispatcherState.hpp

OTHER_FILES += openssl.json

# Platform-specific configuration
linux {
  message(Linux)

  android {
    message(Android)

    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
      message(armeabi-v7a)

      INCLUDEPATH += openssl/android/armv7/include

      LIBS += -Lopenssl/android/armv7/lib -lssl -lcrypto

      debug {
        message(Debug)
        DESTDIR = $$PWD/../../../../build/android/armv7/debug/plugins/cryptography/openssl
      }
      release {
        message(Release)
        DESTDIR = $$PWD/../../../../build/android/armv7/release/plugins/cryptography/openssl
      }
    }

    contains(ANDROID_TARGET_ARCH, arm64-v8a) {
      message(arm64-v8a)

      INCLUDEPATH += openssl/android/arm64_v8a/include

      LIBS += -Lopenssl/android/arm64_v8a/lib -lssl -lcrypto

      debug {
        message(Debug)
        DESTDIR = $$PWD/../../../../build/android/arm64_v8a/debug/plugins/cryptography/openssl
      }
      release {
        message(Release)
        DESTDIR = $$PWD/../../../../build/android/arm64_v8a/release/plugins/cryptography/openssl
      }
    }
  } # End android

  linux-clang {
    message(clang)

    INCLUDEPATH += /usr/local/ssl/include

    LIBS += -L/usr/local/ssl/lib -lssl -lcrypto

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../../../build/linux/clang/x86_64/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/linux/clang/x86_64/release/plugins/cryptography/openssl
    }
  } # End linux-clang

  linux-g++ {
    message(g++)

    INCLUDEPATH += /usr/local/ssl/include

    LIBS += -L/usr/local/ssl/lib -lssl -lcrypto

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../../../build/linux/gcc/x86_64/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/linux/gcc/x86_64/release/plugins/cryptography/openssl
    }
  } # End linux-g++
} # End linux

darwin {
  # Install OpenSSL v3 from Homebrew
  INCLUDEPATH += /usr/local/opt/openssl@3/include

  LIBS += -L/usr/local/opt/openssl@3/lib -lssl -lcrypto

  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  ios {
    message(iOS)
    message(clang)

    CONFIG -= simulator

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../../../build/iOS/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/iOS/release/plugins/cryptography/openssl
    }
  } # End ios

  macos {
    message(macOS)
    message(clang)

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../../../build/macOS/clang/x86_64/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/macOS/clang/x86_64/release/plugins/cryptography/openssl
    }
  } # End macos
} # End darwin

win32 {
  message(Windows)

  win32-msvc {
    message(MSVC)

    LIBS += -ladvapi32 -luser32 -lws2_32

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(x86_64)

      message($${QT_TOOLS}/OpenSSLv3/Win_x64/include)

      INCLUDEPATH += $${QT_TOOLS}/OpenSSLv3/Win_x64/include

      LIBS += -L$${QT_TOOLS}/OpenSSLv3/Win_x64/lib -lssl -lcrypto

      debug {
        message(Debug)
        DESTDIR = $$PWD/../../../../build/windows/msvc/x86_64/debug/plugins/cryptography/openssl
      }
      release {
        message(Release)
        DESTDIR = $$PWD/../../../../build/windows/msvc/x86_64/release/plugins/cryptography/openssl
      }
    }
  } # End win32-msvc
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
