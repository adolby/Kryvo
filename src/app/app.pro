include(../../defaults.pri)

QT += core gui

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
  Constants.cpp \
  Application.cpp \
  Dispatcher.cpp \
  DispatcherState.cpp \
  archive/Archiver.cpp \
  cryptography/Crypto.cpp \
  gui/MainWindow.cpp \
  gui/SettingsFrame.cpp \
  gui/HeaderFrame.cpp \
  gui/FileListFrame.cpp \
  gui/FileListDelegate.cpp \
  gui/ProgressFrame.cpp \
  gui/MessageFrame.cpp \
  gui/OutputFrame.cpp \
  gui/PasswordFrame.cpp \
  gui/ControlButtonFrame.cpp \
  gui/ElidedLabel.cpp \
  gui/flowlayout.cpp \
  gui/SlidingStackedWidget.cpp \
  settings/Settings.cpp

HEADERS += \
  Constants.hpp \
  Application.hpp \
  Dispatcher.hpp \
  DispatcherState.hpp \
  archive/Archiver.hpp \
  cryptography/Crypto.hpp \
  cryptography/CryptoProviderInterface.hpp \
  gui/MainWindow.hpp \
  gui/SettingsFrame.hpp \
  gui/HeaderFrame.hpp \
  gui/FileListFrame.hpp \
  gui/FileListDelegate.hpp \
  gui/ProgressFrame.hpp \
  gui/MessageFrame.hpp \
  gui/OutputFrame.hpp \
  gui/PasswordFrame.hpp \
  gui/ControlButtonFrame.hpp \
  gui/ElidedLabel.hpp \
  gui/flowlayout.h \
  gui/SlidingStackedWidget.hpp \
  settings/Settings.hpp \
  utility/Thread.hpp \
  utility/pimpl.h

# Platform-specific configuration
linux {
  message(Linux)

  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  android {
    message(Android)

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../../resources/android

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/android/armv7/debug/lib/zlib -lz
      DESTDIR = ../../build/android/armv7/debug/Kryvo
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/android/armv7/release/lib/zlib -lz
      DESTDIR = ../../build/android/armv7/release/Kryvo
    }
  } # End android

  linux-clang {
    message(clang)

    QT += widgets

    SOURCES += gui/DesktopMainWindow.cpp
    HEADERS += gui/DesktopMainWindow.hpp

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/lib/zlib -lz
      DESTDIR = ../../build/linux/clang/x86_64/debug/Kryvo
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/lib/zlib -lz
      DESTDIR = ../../build/linux/clang/x86_64/release/Kryvo
    }
  } # End linux-clang

  linux-g++ {
    message(g++)

    QT += widgets

    SOURCES += gui/DesktopMainWindow.cpp
    HEADERS += gui/DesktopMainWindow.hpp

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/lib/zlib -lz
      DESTDIR = ../../build/linux/gcc/x86_64/debug/Kryvo
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/lib/zlib -lz
      DESTDIR = ../../build/linux/gcc/x86_64/release/Kryvo
    }
  } # End linux-g++
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
      DESTDIR = ../../build/iOS/debug/Kryvo
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/iOS/release/lib/zlib -lz
      DESTDIR = ../../build/iOS/release/Kryvo
    }

    LIBS += -framework Foundation -framework CoreFoundation -framework UIKit
  } # End ios

  macos {
    message(macOS)
    message(clang)

#    QMAKE_MAC_SDK = macosx10.13
#    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

    QT += widgets

    SOURCES += gui/DesktopMainWindow.cpp
    HEADERS += gui/DesktopMainWindow.hpp

    QMAKE_TARGET_BUNDLE_PREFIX = app.kryvo
    ICON = ../../resources/icons/kryvo.icns

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/lib/zlib -lz
      DESTDIR = ../../build/macOS/clang/x86_64/debug/Kryvo
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/lib/zlib -lz
      DESTDIR = ../../build/macOS/clang/x86_64/release/Kryvo
    }
  } # End macos
} # End darwin

win32 {
  message(Windows)

  QT += widgets

  SOURCES += gui/DesktopMainWindow.cpp
  HEADERS += gui/DesktopMainWindow.hpp

  win32-g++ {
    message(g++)

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/windows/mingw/x86/debug/lib/zlib -lz
      DESTDIR = ../../build/windows/mingw/x86/debug/Kryvo/
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/windows/mingw/x86/release/lib/zlib -lz
      DESTDIR = ../../build/windows/mingw/x86/release/Kryvo/
    }
  } # End win32-g++

  win32-msvc {
    message(MSVC)

    LIBS += advapi32.lib user32.lib

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(x86_64)

      debug {
        message(Debug)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/lib/zlib -lz
        DESTDIR = ../../build/windows/msvc/x86_64/debug/Kryvo/
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/lib/zlib -lz
        DESTDIR = ../../build/windows/msvc/x86_64/release/Kryvo/
      }
    }
  } # End win32-msvc

  RC_ICONS += ../../resources/icons/kryvo.ico
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc

RESOURCES += ../../resources/assets.qrc
