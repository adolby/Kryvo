QT += core

TARGET = openssl

TEMPLATE = lib

CONFIG += c++14 plugin

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
  OpenSslProvider.cpp \
  ../../../app/Constants.cpp \
  ../../../app/cryptography/CryptoState.cpp

HEADERS += \
  OpenSslProvider.hpp \
  ../../../app/Constants.hpp \
  ../../../app/cryptography/CryptoState.hpp

INCLUDEPATH += \
  ../../../app/ \
  ../../../lib/zlib/

# Platform-specific configuration
linux {
  message(Linux)

  QMAKE_CXXFLAGS += -fstack-protector #-maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  android {
    message(Android)

    debug {
      message(Debug)
      DESTDIR = ../../../../build/android/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = ../../../../build/android/release/plugins/cryptography/openssl
    }
  } # End Android

  linux-clang {
    message(clang)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      DESTDIR = ../../../../build/linux/clang/x86_64/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = ../../../../build/linux/clang/x86_64/release/plugins/cryptography/openssl
    }
  } # End clang

  linux-g++ {
    message(g++)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      DESTDIR = ../../../../build/linux/gcc/x86_64/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = ../../../../build/linux/gcc/x86_64/release/plugins/cryptography/openssl
    }
  } # End g++
} # End Linux

mac {
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
      DESTDIR = ../../../../build/iOS/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = ../../../../build/iOS/release/plugins/cryptography/openssl
    }
  } # End iOS

  macx {
    message(macOS)
    message(clang)

    debug {
      message(Debug)
      DESTDIR = ../../../../build/macOS/clang/x86_64/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = ../../../../build/macOS/clang/x86_64/release/plugins/cryptography/openssl
    }
  } # End macOS
} # End Mac

win32 {
  message(Windows)

  win32-g++ {
    message(g++)

    debug {
      message(Debug)
      DESTDIR = ../../build/windows/mingw/x86_32/debug/plugins/cryptography/openssl
    }
    release {
      message(Release)
      DESTDIR = ../../build/windows/mingw/x86_32/release/plugins/cryptography/openssl
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
        DESTDIR = ../../../../build/windows/msvc/x86_64/debug/plugins/cryptography/openssl
      }
      release {
        message(Release)
        DESTDIR = ../../../../build/windows/msvc/x86_64/release/plugins/cryptography/openssl
      }
    }
  }
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
