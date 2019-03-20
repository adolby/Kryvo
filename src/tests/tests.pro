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
  $$PWD/../app/Constants.cpp \
  $$PWD/../app/DispatcherState.cpp \
  $$PWD/../app/Dispatcher.cpp \
  $$PWD/../app/archive/Archiver.cpp \
  $$PWD/../app/cryptography/Crypto.cpp \
  test_Crypto.cpp \
  test_Archiver.cpp \
  FileOperations.cpp \
  test_FileOperations.cpp \
  main.cpp

HEADERS += \
  $$PWD/../app/Constants.hpp \
  $$PWD/../app/DispatcherState.hpp \
  $$PWD/../app/Dispatcher.hpp \
  $$PWD/../app/archive/Archiver.hpp \
  $$PWD/../app/cryptography/Crypto.hpp \
  $$PWD/../app/utility/Thread.hpp \
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
      LIBS += -L$$PWD/../../build/android/debug/lib/zlib -lz
      DESTDIR = $$PWD/../../build/android/debug/test
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/android/release/lib/zlib -lz
      DESTDIR = $$PWD/../../build/android/release/test
    }
  } # End android

  linux-clang {
    message(clang)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/lib/zlib -lz
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/debug/test
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/lib/zlib -lz
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/release/test
    }
  } # End clang

  linux-g++-64 {
    message(g++ x86_64)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/lib/zlib -lz
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/debug/test
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/lib/zlib -lz
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/release/test
    }
  } # End g++ x86_64
} # End linux

darwin {
  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  ios {
    message(iOS)
    message(clang)

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/iOS/debug/lib/zlib -lz
      DESTDIR = $$PWD/../../build/iOS/debug/test
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/iOS/release/lib/zlib -lz
      DESTDIR = $$PWD/../../build/iOS/release/test
    }
  } # End ios

  macos {
    message(macOS)
    message(clang)

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/lib/zlib -lz
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/debug/test
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/lib/zlib -lz
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/release/test
    }
  } # End macos
} # End darwin

win32 {
  message(Windows)

  win32-g++ {
    message(g++)

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/windows/mingw/x86/debug/lib/zlib -lz
      DESTDIR = $$PWD/../../build/windows/mingw/x86/debug/test
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/windows/mingw/x86/release/lib/zlib -lz
      DESTDIR = $$PWD/../../build/windows/mingw/x86/release/test
    }
  } # End win32-g++

  win32-msvc {
    LIBS += advapi32.lib user32.lib

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(MSVC x86_64)

      debug {
        message(Debug)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/lib/zlib -lz
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/debug/test
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/lib/zlib -lz
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/release/test
      }
    }
  } # End win32-msvc
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc
