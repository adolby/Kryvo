QT += core gui widgets

TARGET = Kryvos

TEMPLATE = app

CONFIG += c++14

SOURCES += \
  main.cpp \
  Kryvos.cpp \
  cryptography/Crypto.cpp \
  gui/MainWindow.cpp \
  gui/SettingsFrame.cpp \
  gui/HeaderFrame.cpp \
  gui/FileListFrame.cpp \
  gui/Delegate.cpp \
  gui/MessageFrame.cpp \
  gui/PasswordFrame.cpp \
  gui/ControlButtonFrame.cpp \
  gui/SlideSwitch.cpp \
  gui/flowlayout.cpp \
  gui/FluidLayout.cpp \
  settings/Settings.cpp

HEADERS += \
  Kryvos.hpp \
  cryptography/Crypto.hpp \
  gui/MainWindow.hpp \
  gui/SettingsFrame.hpp \
  gui/HeaderFrame.hpp \
  gui/FileListFrame.hpp \
  gui/Delegate.hpp \
  gui/MessageFrame.hpp \
  gui/PasswordFrame.hpp \
  gui/ControlButtonFrame.hpp \
  gui/SlideSwitch.hpp \
  gui/flowlayout.h \
  gui/FluidLayout.hpp \
  settings/Settings.hpp \
  utility/make_unique.h \
    cryptography/botan/android/android_to_string.h

# Platform-specific configuration
android-g++ {
  message(Android G++)
  SOURCES += cryptography/botan/android/botan_all.cpp \
             gui/TouchMainWindow.cpp
  HEADERS += cryptography/botan/android/botan_all.h \
             gui/TouchMainWindow.hpp

  ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

  OTHER_FILES += android/AndroidManifest.xml

} else:ios {
  message(iOS)
  SOURCES += cryptography/botan/ios/botan_all.cpp \
             gui/TouchMainWindow.cpp
  HEADERS += cryptography/botan/ios/botan_all.h \
             gui/TouchMainWindow.hpp
} else { # Desktop OS
  SOURCES += gui/DesktopMainWindow.cpp
  HEADERS += gui/DesktopMainWindow.hpp

  linux {
    message(Linux)

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
  }

  macx {
    message(OSX)
    CONFIG += x86
    SOURCES += cryptography/botan/mac/x86_64/botan_all.cpp
    HEADERS += cryptography/botan/mac/x86_64/botan_all.h
    QMAKE_MAC_SDK = macosx10.9
    ICON = resources/mac/icon/Kryvos.icns
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

    RC_FILE = resources/windows/rc/kryvos.rc
  }
}

RESOURCES += resources/kryvos.qrc

QMAKE_CXXFLAGS += -fstack-protector
QMAKE_LFLAGS += -fstack-protector
