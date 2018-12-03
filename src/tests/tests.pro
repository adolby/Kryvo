include(../../defaults.pri)

TARGET = tests

TEMPLATE = app

CONFIG += qt console warn_on depend_includepath testcase
CONFIG += c++14

QT += testlib
QT -= gui

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
  ../app/Constants.cpp \
  ../app/DispatcherState.cpp \
  ../app/Dispatcher.cpp \
  ../app/archive/Archiver.cpp \
  ../app/cryptography/Crypto.cpp \
  test_Crypto.cpp \
  test_Archiver.cpp \
  FileOperations.cpp \
  test_FileOperations.cpp \
  main.cpp

HEADERS += \
  ../app/Constants.hpp \
  ../app/DispatcherState.hpp \
  ../app/Dispatcher.hpp \
  ../app/archive/Archiver.hpp \
  ../app/cryptography/Crypto.hpp \
  ../app/utility/Thread.hpp \
  FileOperations.hpp

LIBS += -lz

# Platform-specific configuration
linux {
  message(Linux)

  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  android {
    message(Android)

#    HEADERS += src/libs/botan/android/android_to_string.h

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../resources/android

    debug {
      message(Debug)
      DESTDIR = ../../build/android/debug/test/
    }
    release {
      message(Release)
      DESTDIR = ../../build/android/release/test/
    }
  } # End android

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
} # End linux

darwin {
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
  } # End ios

  macos {
    message(macOS)
    message(clang)

#    QMAKE_MAC_SDK = macosx10.13
#    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

    debug {
      message(Debug)
      DESTDIR = ../../build/macOS/clang/x86_64/debug/test/
    }
    release {
      message(Release)
      DESTDIR = ../../build/macOS/clang/x86_64/release/test/
    }
  } # End macos
} # End darwin

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
  } # End win32-g++

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
  } # End win32-msvc
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc
