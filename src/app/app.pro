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
  Application.cpp \
  archive/Archiver.cpp \
  cryptography/Crypto.cpp \
  cryptography/CryptoState.cpp \
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
  settings/Settings.cpp \
  utility/Thread.cpp

HEADERS += \
  constants.h \
  Application.hpp \
  archive/Archiver.hpp \
  cryptography/Crypto.hpp \
  cryptography/CryptoState.hpp \
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
      DESTDIR = ../../build/android/debug/Kryvo
    }
    release {
      message(Release)
      DESTDIR = ../../build/android/release/Kryvo
    }
  }

  linux-clang {
    message(clang)

    QT += widgets

    SOURCES += gui/DesktopMainWindow.cpp
    HEADERS += gui/DesktopMainWindow.hpp

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      DESTDIR = ../../build/linux/clang/x86_64/debug/Kryvo
    }
    release {
      message(Release)
      DESTDIR = ../../build/linux/clang/x86_64/release/Kryvo
    }
  }

  linux-g++ {
    message(g++)

    QT += widgets

    SOURCES += gui/DesktopMainWindow.cpp
    HEADERS += gui/DesktopMainWindow.hpp

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      DESTDIR = ../../build/linux/gcc/x86_64/debug/Kryvo
    }
    release {
      message(Release)
      DESTDIR = ../../build/linux/gcc/x86_64/release/Kryvo
    }
  }
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
      DESTDIR = ../../build/iOS/debug/Kryvo
    }
    release {
      message(Release)
      DESTDIR = ../../build/iOS/release/Kryvo
    }
  }

  macx {
    message(macOS)
    message(clang)

    QT += widgets

    SOURCES += gui/DesktopMainWindow.cpp
    HEADERS += gui/DesktopMainWindow.hpp

    QMAKE_TARGET_BUNDLE_PREFIX = io.kryvo
    ICON = ../../resources/icons/kryvo.icns

    debug {
      message(Debug)
      DESTDIR = ../../build/macOS/clang/x86_64/debug/Kryvo
    }
    release {
      message(Release)
      DESTDIR = ../../build/macOS/clang/x86_64/release/Kryvo
    }
  }
}

win32 {
  message(Windows)

  QT += widgets

  SOURCES += gui/DesktopMainWindow.cpp
  HEADERS += gui/DesktopMainWindow.hpp

  win32-msvc2015 {
    LIBS += advapi32.lib user32.lib

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(MSVC x86_64)

      debug {
        message(Debug)
        DESTDIR = ../../build/windows/msvc/x86_64/debug/Kryvo
      }
      release {
        message(Release)
        DESTDIR = ../../build/windows/msvc/x86_64/release/Kryvo
      }
    } else {
      message(MSVC x86)

      debug {
        message(Debug)
        DESTDIR = ../../build/windows/msvc/x86/debug/Kryvo
      }
      release {
        message(Release)
        DESTDIR = ../../build/windows/msvc/x86/release/Kryvo
      }
    }
  }

  RC_ICONS += ../../resources/icons/kryvo.ico
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc

RESOURCES += ../../resources/assets.qrc
