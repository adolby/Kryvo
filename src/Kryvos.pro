# Kryvos - Encrypts and decrypts files.
# Copyright (C) 2014, 2015 Andrew Dolby

# This file is part of Kryvos.

# Kryvos is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Kryvos is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Kryvos.  If not, see <http://www.gnu.org/licenses/>.

# Contact : andrewdolby@gmail.com

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
  cryptography/botan/android/android_to_string.h \
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
  utility/make_unique.h

# Platform-specific configuration
android-g++ {
  message(Android G++)

  # You'll need to place your Boost path here.
  INCLUDEPATH += $$(HOME)/Boost/boost_1_58_0/

  SOURCES += cryptography/botan/android/botan_all.cpp \
             gui/TouchMainWindow.cpp
  HEADERS += cryptography/botan/android/botan_all.h \
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

    linux-g++-64 {
      message(Linux G++ 64)
      SOURCES += cryptography/botan/linux/x86_64/botan_all.cpp
      HEADERS += cryptography/botan/linux/x86_64/botan_all.h

      debug {
        message(Debug)
        DESTDIR = ../build/linux/x64/debug/
      }
      release {
        message(Release)
        DESTDIR = ../build/linux/x64/release/
      }
    }

    linux-g++-32 {
      message(Linux G++ 32)

      SOURCES += cryptography/botan/linux/x86/botan_all.cpp
      HEADERS += cryptography/botan/linux/x86/botan_all.h

      debug {
        message(Debug)
        DESTDIR = ../build/linux/x86/debug/
      }
      release {
        message(Release)
        DESTDIR = ../build/linux/x86/release/
      }
    } # End linux-g++-32

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"
  } # End Linux

  macx {
    message(Mac OS X)

    CONFIG += x86
    CONFIG -= c++14
    CONFIG += c++11

    SOURCES += cryptography/botan/mac/x86_64/botan_all.cpp
    HEADERS += cryptography/botan/mac/x86_64/botan_all.h

    QMAKE_MAC_SDK = macosx10.9
    ICON = resources/mac/icon/Kryvos.icns

    debug {
      message(Debug)
      DESTDIR = ../build/macx/x86/debug/
    }
    release {
      message(Release)
      DESTDIR = ../build/macx/x86/release/
    }
  }

  win32 {
    win32-g++ {
      contains(QT_ARCH, x86_64) {
        message(Windows x64 G++)

        SOURCES += cryptography/botan/windows/x64/botan_all.cpp
        HEADERS += cryptography/botan/windows/x64/botan_all.h

        debug {
          message(Debug)
          DESTDIR = ../build/win/x64/debug/
        }
        release {
          message(Release)
          DESTDIR = ../build/win/x64/release/
        }
      } else {
        message(Windows x86 G++)
        SOURCES += cryptography/botan/windows/x86/botan_all.cpp
        HEADERS += cryptography/botan/windows/x86/botan_all.h

        debug {
          message(Debug)
          DESTDIR = ../build/win/x86/debug/
        }
        release {
          message(Release)
          DESTDIR = ../build/win/x86/release/
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

QMAKE_CXXFLAGS += -fstack-protector
QMAKE_LFLAGS += -fstack-protector
