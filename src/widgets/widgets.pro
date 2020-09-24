include(../../defaults.pri)

QT += core gui widgets

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
  MainWindow.cpp \
  SettingsFrame.cpp \
  HeaderFrame.cpp \
  FileListFrame.cpp \
  FileListDelegate.cpp \
  ProgressFrame.cpp \
  MessageFrame.cpp \
  OutputFrame.cpp \
  PasswordFrame.cpp \
  ControlButtonFrame.cpp \
  ElidedLabel.cpp \
  flowlayout.cpp \
  SlidingStackedWidget.cpp

HEADERS += \
  Application.hpp \
  MainWindow.hpp \
  SettingsFrame.hpp \
  HeaderFrame.hpp \
  FileListFrame.hpp \
  FileListDelegate.hpp \
  ProgressFrame.hpp \
  MessageFrame.hpp \
  OutputFrame.hpp \
  PasswordFrame.hpp \
  ControlButtonFrame.hpp \
  ElidedLabel.hpp \
  flowlayout.h \
  SlidingStackedWidget.hpp

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
        DESTDIR = $$PWD/../../build/android/armv7/debug/widgets
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/android/armv7/release/core -lcore
        LIBS += -L$$PWD/../../build/android/armv7/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/armv7/release/plugins/cryptography/botan -lbotan
        DESTDIR = $$PWD/../../build/android/armv7/release/widgets
      }
    }

    contains(ANDROID_TARGET_ARCH, arm64-v8a) {
      message(arm64-v8a)

      debug {
        message(Debug)
        LIBS += -L$$PWD/../../build/android/arm64_v8a/debug/core -lcore
        LIBS += -L$$PWD/../../build/android/arm64_v8a/debug/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/arm64_v8a/debug/plugins/cryptography/botan -lbotan
        DESTDIR = $$PWD/../../build/android/arm64_v8a/debug/widgets
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/android/arm64_v8a/release/core -lcore
        LIBS += -L$$PWD/../../build/android/arm64_v8a/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/arm64_v8a/release/plugins/cryptography/botan -lbotan
        DESTDIR = $$PWD/../../build/android/arm64_v8a/release/widgets
      }
    }
  } # End android

  linux-clang {
    message(clang)

    SOURCES += DesktopMainWindow.cpp
    HEADERS += DesktopMainWindow.hpp

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/debug/widgets
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/release/widgets
    }
  } # End linux-clang

  linux-g++ {
    message(g++)

    SOURCES += DesktopMainWindow.cpp
    HEADERS += DesktopMainWindow.hpp

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/debug/widgets
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/release/widgets
    }
  } # End linux-g++
} # End linux

darwin {
  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  macos {
    message(macOS)
    message(clang)

    SOURCES += DesktopMainWindow.cpp
    HEADERS += DesktopMainWindow.hpp

    QMAKE_TARGET_BUNDLE_PREFIX = app.kryvo
    ICON = $$PWD/../../resources/icons/kryvo.icns

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/debug/widgets
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/release/widgets
    }
  } # End macos
} # End darwin

win32 {
  message(Windows)

  SOURCES += DesktopMainWindow.cpp
  HEADERS += DesktopMainWindow.hpp

  win32-g++ {
    message(g++)

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/debug/core -lcore
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/windows/mingw/x86_32/debug/widgets
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/release/core -lcore
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/windows/mingw/x86_32/release/widgets
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
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/debug/widgets
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/core -lcore
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/plugins/cryptography/botan -lbotan
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/release/widgets
      }
    }
  } # End win32-msvc

  RC_ICONS += $$PWD/../../resources/icons/kryvo.ico
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc

RESOURCES += \
  $$PWD/../../resources/images.qrc \
  $$PWD/../../resources/stylesheets.qrc
