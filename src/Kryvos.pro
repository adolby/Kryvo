QT += core gui widgets

TARGET = Kryvos

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
  Kryvos.cpp \
  cryptography/Crypto.cpp \
  gui/MainWindow.cpp \
  gui/SettingsFrame.cpp \
  gui/HeaderFrame.cpp \
  gui/FileListFrame.cpp \
  gui/FileListDelegate.cpp \
  gui/MessageFrame.cpp \
  gui/PasswordFrame.cpp \
  gui/ControlButtonFrame.cpp \
  gui/SlideSwitch.cpp \
  gui/flowlayout.cpp \
  gui/FluidLayout.cpp \
  gui/SlidingStackedWidget.cpp \
  settings/Settings.cpp

HEADERS += \
  Kryvos.hpp \
  cryptography/Crypto.hpp \
  gui/MainWindow.hpp \
  gui/SettingsFrame.hpp \
  gui/HeaderFrame.hpp \
  gui/FileListFrame.hpp \
  gui/FileListDelegate.hpp \
  gui/MessageFrame.hpp \
  gui/PasswordFrame.hpp \
  gui/ControlButtonFrame.hpp \
  gui/SlideSwitch.hpp \
  gui/flowlayout.h \
  gui/FluidLayout.hpp \
  gui/SlidingStackedWidget.hpp \
  settings/Settings.hpp \
  utility/pimpl_impl.h \
  utility/pimpl.h

# Platform-specific configuration
android-g++ {
  message(Android G++)

  # You'll need to place your Boost path here.
  INCLUDEPATH += $$(HOME)/Boost/boost_1_58_0/

  SOURCES += cryptography/botan/android/botan_all.cpp \
             gui/TouchMainWindow.cpp
  HEADERS += cryptography/botan/android/botan_all.h \
             cryptography/botan/android/android_to_string.h \
             gui/TouchMainWindow.hpp

  ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

  OTHER_FILES += android/AndroidManifest.xml

  debug {
    message(Debug)
    DESTDIR = ../build/android/debug/
  }
  release {
    message(Release)
    DESTDIR = ../build/android/release/
  }
} else:ios {
  message(iOS)

  SOURCES += cryptography/botan/ios/botan_all.cpp \
             gui/TouchMainWindow.cpp
  HEADERS += cryptography/botan/ios/botan_all.h \
             gui/TouchMainWindow.hpp

  debug {
    message(Debug)
    DESTDIR = ../build/ios/debug/
  }
  release {
    message(Release)
    DESTDIR = ../build/ios/release/
  }
} else { # Desktop OS
  SOURCES += gui/DesktopMainWindow.cpp
  HEADERS += gui/DesktopMainWindow.hpp

  linux {
    message(Linux)
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    linux-clang {
      message(Linux Clang x86_64)
      SOURCES += cryptography/botan/linux/clang/x86_64/botan_all.cpp
      HEADERS += cryptography/botan/linux/clang/x86_64/botan_all.h

      debug {
        message(Debug)
        DESTDIR = ../build/linux/clang/x86_64/debug/
      }
      release {
        message(Release)
        DESTDIR = ../build/linux/clang/x86_64/release/
      }
    }

    linux-g++-64 {
      message(Linux G++ x86_64)
      QMAKE_CXXFLAGS += -fstack-protector
      QMAKE_LFLAGS += -fstack-protector

      SOURCES += cryptography/botan/linux/gcc/x86_64/botan_all.cpp
      HEADERS += cryptography/botan/linux/gcc/x86_64/botan_all.h

      debug {
        message(Debug)
        DESTDIR = ../build/linux/gcc/x86_64/debug/
      }
      release {
        message(Release)
        DESTDIR = ../build/linux/gcc/x86_64/release/
      }
    }

    linux-g++-32 {
      message(Linux G++ x86)
      QMAKE_CXXFLAGS += -fstack-protector
      QMAKE_LFLAGS += -fstack-protector

      SOURCES += cryptography/botan/linux/gcc/x86/botan_all.cpp
      HEADERS += cryptography/botan/linux/gcc/x86/botan_all.h

      debug {
        message(Debug)
        DESTDIR = ../build/linux/gcc/x86/debug/
      }
      release {
        message(Release)
        DESTDIR = ../build/linux/gcc/x86/release/
      }
    } # End linux-g++-32
  } # End Linux

  macx {
    message(Mac OS X)

    # Manually set c++1y until config += c++14 is fixed for OS X
    CONFIG -= c++14
    QMAKE_CXXFLAGS += -std=c++1y

    SOURCES += cryptography/botan/mac/clang/x86_64/botan_all.cpp
    HEADERS += cryptography/botan/mac/clang/x86_64/botan_all.h

    ICON = resources/mac/icon/Kryvos.icns

    debug {
      message(Debug)
      DESTDIR = ../build/macx/clang/x86_64/debug/
    }
    release {
      message(Release)
      DESTDIR = ../build/macx/clang/x86_64/release/
    }
  }

  win32 {
    win32-g++ {
      QMAKE_CXXFLAGS += -fstack-protector
      QMAKE_LFLAGS += -fstack-protector

      contains(QT_ARCH, x86_64) {
        message(Windows x64 G++)

        SOURCES += cryptography/botan/windows/mingw/x64/botan_all.cpp
        HEADERS += cryptography/botan/windows/mingw/x64/botan_all.h \
                   cryptography/botan/windows/mingw/x64/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../build/win/mingw/x64/debug/
        }
        release {
          message(Release)
          DESTDIR = ../build/win/mingw/x64/release/
        }
      } else {
        message(Windows x86 G++)
        SOURCES += cryptography/botan/windows/mingw/x86/botan_all.cpp
        HEADERS += cryptography/botan/windows/mingw/x86/botan_all.h \
                   cryptography/botan/windows/mingw/x86/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../build/win/mingw/x86/debug/
        }
        release {
          message(Release)
          DESTDIR = ../build/win/mingw/x86/release/
        }
      }
    }

    win32-msvc {
      contains(QT_ARCH, x86_64) {
        message(Windows x64 MSVC 2015)

        SOURCES += cryptography/botan/windows/msvc/botan_all.cpp
        HEADERS += cryptography/botan/windows/msvc/x64/botan_all.h \
                   cryptography/botan/windows/msvc/x64/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../build/win/msvc-2015/x64/debug/
        }
        release {
          message(Release)
          DESTDIR = ../build/win/msvc-2015/x64/release/
        }
      } else {
        message(Windows x86 MSVC 2015)
        SOURCES += cryptography/botan/windows/msvc/x86/botan_all.cpp
        HEADERS += cryptography/botan/windows/msvc/x86/botan_all.h \
                   cryptography/botan/windows/msvc/x86/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../build/win/x86/msvc-2015/debug/
        }
        release {
          message(Release)
          DESTDIR = ../build/win/msvc-2015/x86/release/
        }
      }
    }

    RC_ICONS += resources/windows/icon/kryvos.ico
  } # End win32
} # End desktop

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/qrc

RESOURCES += resources/kryvos.qrc
