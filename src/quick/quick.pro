include(../../defaults.pri)

QT += core gui qml quick quickcontrols2

TARGET = Kryvo

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

  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  android {
    message(Android)

    QT += quick

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../../resources/android

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/android/armv7/debug/core -lcore
      LIBS += -L$$PWD/../../build/android/armv7/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/android/armv7/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/android/armv7/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/android/armv7/release/core -lcore
      LIBS += -L$$PWD/../../build/android/armv7/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/android/armv7/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/android/armv7/release/quick
    }
  } # End android

  linux-clang {
    message(clang)

    QT += quick

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/release/quick
    }
  } # End linux-clang

  linux-g++-64 {
    message(g++ x86_64)

    QT += quick

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/release/quick
    }
  } # End linux-g++-64
} # End linux

darwin {
  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  ios {
    message(iOS)
    message(clang)

    QT += quick

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/iOS/debug/core -lcore
      LIBS += -L$$PWD/../../build/iOS/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/iOS/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/iOS/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/iOS/release/core -lcore
      LIBS += -L$$PWD/../../build/iOS/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/iOS/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/iOS/release/quick
    }

    LIBS += -framework Foundation -framework CoreFoundation -framework UIKit
  } # End ios

  macos {
    message(macOS)
    message(clang)

    QT += quick

    QMAKE_TARGET_BUNDLE_PREFIX = app.kryvo
    ICON = $$PWD/../../resources/icons/kryvo.icns

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/release/quick
    }
  } # End macos
} # End darwin

win32 {
  message(Windows)

  QT += quick

  win32-g++ {
    message(g++)

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/windows/mingw/x86/debug/core -lcore
      LIBS += -L$$PWD/../../build/windows/mingw/x86/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/windows/mingw/x86/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/windows/mingw/x86/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/windows/mingw/x86/release/core -lcore
      LIBS += -L$$PWD/../../build/windows/mingw/x86/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/windows/mingw/x86/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/windows/mingw/x86/release/quick
    }
  } # End win32-g++

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
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/debug/quick
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/core -lcore
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/plugins/cryptography/botan -lbotan
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/release/quick
      }
    }
  } # End win32-msvc

  RC_ICONS += $$PWD/../../resources/icons/kryvo.ico
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc

RESOURCES += $$PWD/../../resources/images.qrc $$PWD/quick.qrc
