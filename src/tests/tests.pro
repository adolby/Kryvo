QT += core testlib
QT -= gui

TARGET = CryptoTests

TEMPLATE = app

CONFIG += c++14

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

INCLUDEPATH += $$PWD/../app/

SOURCES += \
  ../app/Constants.cpp \
  ../app/cryptography/Crypto.cpp \
  ../app/cryptography/CryptoState.cpp \
  test_Crypto.cpp \
#  test_Archiver.cpp \
  FileOperations.cpp

HEADERS += \
  ../app/Constants.hpp \
  ../app/cryptography/Crypto.hpp \
  ../app/cryptography/CryptoState.hpp \
  test_Crypto.hpp \
#  test_Archiver.hpp \
  FileOperations.h

LIBS += -lz

# Platform-specific configuration
linux {
  message(Linux)

  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  android {
    message(Android)

    # You'll need to place your Boost path here.
    INCLUDEPATH += $$(HOME)/Boost/boost_1_58_0/

    HEADERS += src/libs/botan/android/android_to_string.h

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../resources/android

    debug {
      message(Debug)
      DESTDIR = ../../build/android/debug/test/
    }
    release {
      message(Release)
      DESTDIR = ../../build/android/release/test/
    }
  } # End Android

  linux-clang {
    message(clang)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      DESTDIR = ../../build/linux/clang/x86_64/debug/test/
    }
    release {
      message(Release)
      DESTDIR = ../../build/linux/clang/x86_64/release/test/
    }
  } # End clang

  linux-g++ {
    message(g++)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      DESTDIR = ../../build/linux/gcc/x86_64/debug/test/
    }
    release {
      message(Release)
      DESTDIR = ../../build/linux/gcc/x86_64/release/test/
    }
  } # End g++
} # End Linux

mac {
  QMAKE_MAC_SDK = macosx10.13
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  ios {
    message(iOS)
    message(clang)

    debug {
      message(Debug)
      DESTDIR = ../../build/iOS/debug/test/
    }
    release {
      message(Release)
      DESTDIR = ../../build/iOS/release/test/
    }
  } # End iOS

  macx {
    message(macOS)
    message(clang)

    debug {
      message(Debug)
      DESTDIR = ../../build/macOS/clang/x86_64/debug/test/
    }
    release {
      message(Release)
      DESTDIR = ../../build/macOS/clang/x86_64/release/test/
    }
  } # End macOS
} # End Mac

win32 {
  message(Windows)

  win32-g++ {
    message(g++)

    debug {
      message(Debug)
      DESTDIR = ../../build/windows/mingw/x86/debug/test/
    }
    release {
      message(Release)
      DESTDIR = ../../build/windows/mingw/x86/release/test/
    }
  }

  win32-msvc {
    LIBS += advapi32.lib user32.lib

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(MSVC x86_64)

      debug {
        message(Debug)
        DESTDIR = ../../build/windows/msvc/x86_64/debug/test/
      }
      release {
        message(Release)
        DESTDIR = ../../build/windows/msvc/x86_64/release/test/
      }
    }
  }
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc
