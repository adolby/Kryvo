QT += core gui widgets

TARGET = Kryvos

TEMPLATE = app

CONFIG += c++11

SOURCES += \
  main.cpp \
  Kryvos.cpp \
  cryptography/Crypto.cpp \
  gui/MainWindow.cpp \
  gui/HeaderFrame.cpp \
  gui/FileListFrame.cpp \
  gui/Delegate.cpp \
  gui/MessageFrame.cpp \
  gui/PasswordFrame.cpp \
  gui/ControlButtonFrame.cpp \
  settings/Settings.cpp \
  utility/flowlayout.cpp

HEADERS += \
  Kryvos.hpp \
  cryptography/Crypto.hpp \
  gui/MainWindow.hpp \
  gui/HeaderFrame.hpp \
  gui/FileListFrame.hpp \
  gui/Delegate.hpp \
  gui/MessageFrame.hpp \
  gui/PasswordFrame.hpp \
  gui/ControlButtonFrame.hpp \
  settings/Settings.hpp \
  utility/make_unique.h \
  utility/flowlayout.h

# Platform-specific configuration
android-g++ {
  message(Android G++)
  SOURCES += gui/android/AndroidMainWindow.cpp \
             cryptography/botan/android/botan_all.cpp
  HEADERS += gui/android/AndroidMainWindow.hpp \
             cryptography/botan/android/botan_all.h
} else:ios {
  message(iOS)
  SOURCES += gui/ios/MainWindow.hpp \
             cryptography/botan/ios/botan_all.cpp
  HEADERS += gui/ios/MainWindow.hpp \
             cryptography/botan/android/botan_all.h
} else { # Desktop OS
  SOURCES += gui/DesktopMainWindow.cpp
  HEADERS += gui/DesktopMainWindow.hpp

  linux-g++-64 {
    message(Linux G++ 64)
    SOURCES += cryptography/botan/linux/x86_64/botan_all.cpp
    HEADERS += cryptography/botan/linux/x86_64/botan_all.h
  }

  linux-g++-32 {
    message(Linux G++ 32)
    SOURCES += cryptography/botan/linux/x86/botan_all.cpp
    HEADERS += cryptography/botan/linux/x86/botan_all.h
  }

  macx {
    message(OSX)
  }

  win32 {
    win32-g++ {
      contains(QT_ARCH, x86_64) {
        message(Windows x64 G++)
        SOURCES += cryptography/botan/windows/x64/botan_all.cpp
        HEADERS += cryptography/botan/windows/x64/botan_all.h
      } else {
        message(Windows x86 G++)
        SOURCES += cryptography/botan/windows/x86/botan_all.cpp
        HEADERS += cryptography/botan/windows/x86/botan_all.h
      }
    }
  }
}

RESOURCES += resources/kryvos.qrc

QMAKE_CXXFLAGS += -fstack-protector
QMAKE_LFLAGS += -fstack-protector

RC_FILE = resources/rc/kryvos.rc

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
  android/AndroidManifest.xml
