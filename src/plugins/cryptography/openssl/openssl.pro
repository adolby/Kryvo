include(../../../../defaults.pri)

QT += core

TARGET = openssl

TEMPLATE = lib

CONFIG += plugin static c++14

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

# Platform-specific configuration
linux {
  message(Linux)

  android {
    message(Android)

    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
      message(armeabi-v7a)

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

  linux-g++-64 {
    message(g++ x86_64)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../../../build/linux/gcc/x86_64/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/linux/gcc/x86_64/release/plugins/cryptography/openssl
    }
  } # End linux-g++-64
} # End linux

darwin {
#  QMAKE_MAC_SDK = macosx10.13
#  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

  QMAKE_CXXFLAGS += -fstack-protector #-maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

#  LIBS += -framework Security

  ios {
    message(iOS)
    message(clang)

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../../../build/iOS/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/iOS/release/plugins/cryptography/openssl
    }
  } # End ios

  macx {
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

  win32-g++ {
    message(g++)

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../build/windows/mingw/x86_32/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../build/windows/mingw/x86_32/release/plugins/cryptography/openssl
    }
  }

  win32-msvc {
    message(MSVC)

    LIBS += advapi32.lib user32.lib ws2_32.lib

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(x86_64)

      debug {
        message(Debug)
        DESTDIR = $$PWD/../../../../build/windows/msvc/x86_64/debug/plugins/cryptography/openssl
      }
      release {
        message(Release)
        DESTDIR = $$PWD/../../../../build/windows/msvc/x86_64/release/plugins/cryptography/openssl
      }
    }
  }
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
