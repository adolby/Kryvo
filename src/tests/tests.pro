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

  debug {
    DESTDIR = ../../build/android/x64/debug/test/
  }
  release {
    DESTDIR = ../../build/android/x64/release/test/
  }
} else:ios {
  message(iOS)
  SOURCES += ../cryptography/botan/ios/botan_all.cpp
  HEADERS += ../cryptography/botan/ios/botan_all.h

  debug {
    DESTDIR = ../../build/ios/debug/test/
  }
  release {
    DESTDIR = ../../build/ios/release/test/
  }
} else { # Desktop OS
  linux {
    linux-g++-64 {
      message(Linux G++ 64)
      SOURCES += ../cryptography/botan/linux/x86_64/botan_all.cpp
      HEADERS += ../cryptography/botan/linux/x86_64/botan_all.h

      debug {
        DESTDIR = ../../build/linux/x64/debug/test/
      }
      release {
        DESTDIR = ../../build/linux/x64/release/test/
      }
    }

    linux-g++-32 {
      message(Linux G++ 32)
      SOURCES += ../cryptography/botan/linux/x86/botan_all.cpp
      HEADERS += ../cryptography/botan/linux/x86/botan_all.h

      debug {
        DESTDIR = ../../build/linux/x86/debug/test/
      }
      release {
        DESTDIR = ../../build/linux/x86/release/test/
      }
    }
  }

  macx {
    message(OS X)
    CONFIG += x86
    SOURCES += ../cryptography/botan/mac/x86_64/botan_all.cpp
    HEADERS += ../cryptography/botan/mac/x86_64/botan_all.h
    QMAKE_MAC_SDK = macosx10.9

    debug {
      DESTDIR = ../../build/macx/x64/debug/test/
    }
    release {
      DESTDIR = ../../build/macx/x64/release/test/
    }
  }

  win32 {
    win32-g++ {
      contains(QT_ARCH, x86_64) {
        message(Windows x64 G++)
        SOURCES += ../cryptography/botan/windows/x64/botan_all.cpp
        HEADERS += ../cryptography/botan/windows/x64/botan_all.h

        debug {
          DESTDIR = ../../build/win/x64/debug/test/
        }
        release {
          DESTDIR = ../../build/win/x64/release/test/
        }
      } else {
        message(Windows x86 G++)
        SOURCES += ../cryptography/botan/windows/x86/botan_all.cpp
        HEADERS += ../cryptography/botan/windows/x86/botan_all.h

        debug {
          DESTDIR = ../../build/win/x86/debug/test/
        }
        release {
          DESTDIR = ../../build/win/x86/release/test/
        }
      }
    }
  }
}

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/qrc

QMAKE_CXXFLAGS += -fstack-protector
QMAKE_LFLAGS += -fstack-protector
