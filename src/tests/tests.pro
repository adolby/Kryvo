QT += core testlib

TARGET = tests

TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += ..

# Input
SOURCES += \
  ../cryptography/Crypto.cpp \
  test_Crypto.cpp

HEADERS += \
  ../cryptography/Crypto.hpp \
  test_Crypto.hpp

# Platform-specific configuration
android-g++ {
  message(Android G++)
  SOURCES += ../cryptography/botan/android/botan_all.cpp
  HEADERS += ../cryptography/botan/android/botan_all.h
} else:ios {
  message(iOS)
  SOURCES += ../cryptography/botan/ios/botan_all.cpp
  HEADERS += ../cryptography/botan/ios/botan_all.h
} else { # Desktop OS
  linux {
    linux-g++-64 {
      message(Linux G++ 64)
      SOURCES += ../cryptography/botan/linux/x86_64/botan_all.cpp
      HEADERS += ../cryptography/botan/linux/x86_64/botan_all.h
    }

    linux-g++-32 {
      message(Linux G++ 32)
      SOURCES += ../cryptography/botan/linux/x86/botan_all.cpp
      HEADERS += ../cryptography/botan/linux/x86/botan_all.h
    }
  }

  macx {
    message(OSX)
    CONFIG += x86
    SOURCES += ../cryptography/botan/mac/x86_64/botan_all.cpp
    HEADERS += ../cryptography/botan/mac/x86_64/botan_all.h
    QMAKE_MAC_SDK = macosx10.9
  }

  win32 {
    win32-g++ {
      contains(QT_ARCH, x86_64) {
        message(Windows x64 G++)
        SOURCES += ../cryptography/botan/windows/x64/botan_all.cpp
        HEADERS += ../cryptography/botan/windows/x64/botan_all.h
      } else {
        message(Windows x86 G++)
        SOURCES += ../cryptography/botan/windows/x86/botan_all.cpp
        HEADERS += ../cryptography/botan/windows/x86/botan_all.h
      }
    }
  }
}
