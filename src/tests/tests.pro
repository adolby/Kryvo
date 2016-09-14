QT += core testlib

TARGET = CryptoTests

TEMPLATE = app

CONFIG += c++14

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

  # You'll need to place your Boost path here.
  INCLUDEPATH += $$(HOME)/Boost/boost_1_58_0/

  SOURCES += ../cryptography/botan/android/botan_all.cpp
  HEADERS += ../cryptography/botan/android/botan_all.h \
             ../cryptography/botan/android/android_to_string.h

  ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

  OTHER_FILES += ../android/AndroidManifest.xml

  debug {
    message(Debug)
    DESTDIR = ../../build/android/debug/test
  }
  release {
    message(Release)
    DESTDIR = ../../build/android/release/test
  }
} else:ios {
  message(iOS)

  SOURCES += ../cryptography/botan/ios/botan_all.cpp
  HEADERS += ../cryptography/botan/ios/botan_all.h

  debug {
    message(Debug)
    DESTDIR = ../../build/ios/debug/test
  }
  release {
    message(Release)
    DESTDIR = ../../build/ios/release/test
  }
} else { # Desktop OS
  linux {
    message(Linux)
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    linux-clang {
      message(Linux Clang x86_64)
      SOURCES += ../cryptography/botan/linux/clang/x86_64/botan_all.cpp
      HEADERS += ../cryptography/botan/linux/clang/x86_64/botan_all.h

      debug {
        message(Debug)
        DESTDIR = ../../build/linux/clang/x86_64/debug/test
      }
      release {
        message(Release)
        DESTDIR = ../../build/linux/clang/x86_64/release/test
      }
    }

    linux-g++-64 {
      message(Linux G++ x86_64)
      QMAKE_CXXFLAGS += -fstack-protector
      QMAKE_LFLAGS += -fstack-protector

      SOURCES += ../cryptography/botan/linux/gcc/x86_64/botan_all.cpp
      HEADERS += ../cryptography/botan/linux/gcc/x86_64/botan_all.h

      debug {
        message(Debug)
        DESTDIR = ../../build/linux/gcc/x86_64/debug/test
      }
      release {
        message(Release)
        DESTDIR = ../../build/linux/gcc/x86_64/release/test
      }
    }

    linux-g++-32 {
      message(Linux G++ x86)
      QMAKE_CXXFLAGS += -fstack-protector
      QMAKE_LFLAGS += -fstack-protector

      SOURCES += ../cryptography/botan/linux/gcc/x86/botan_all.cpp
      HEADERS += ../cryptography/botan/linux/gcc/x86/botan_all.h

      debug {
        message(Debug)
        DESTDIR = ../../build/linux/gcc/x86/debug/test
      }
      release {
        message(Release)
        DESTDIR = ../../build/linux/gcc/x86/release/test
      }
    } # End linux-g++-32
  } # End Linux

  macx {
    message(Mac OS X)

    # Manually set c++1y until config += c++14 is fixed for OS X
    #QMAKE_CXXFLAGS += -std=c++1y

    CONFIG += c++11

    SOURCES += ../cryptography/botan/mac/clang/x86_64/botan_all.cpp
    HEADERS += ../cryptography/botan/mac/clang/x86_64/botan_all.h

    debug {
      message(Debug)
      DESTDIR = ../../build/macx/clang/x86_64/debug/test
    }
    release {
      message(Release)
      DESTDIR = ../../build/macx/clang/x86_64/release/test
    }
  }

  win32 {
    win32-g++ {
      QMAKE_CXXFLAGS += -fstack-protector
      QMAKE_LFLAGS += -fstack-protector

      contains(QT_ARCH, x86_64) {
        message(Windows x86_64 G++)

        SOURCES += ../cryptography/botan/windows/mingw/x86_64/botan_all.cpp
        HEADERS += ../cryptography/botan/windows/mingw/x86_64/botan_all.h \
                   ../cryptography/botan/windows/mingw/x86_64/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../../build/win/mingw/x86_64/debug/test
        }
        release {
          message(Release)
          DESTDIR = ../../build/win/mingw/x86_64/release/test
        }
      } else {
        message(Windows x86 G++)
        SOURCES += ../cryptography/botan/windows/mingw/x86/botan_all.cpp
        HEADERS += ../cryptography/botan/windows/mingw/x86/botan_all.h \
                   ../cryptography/botan/windows/mingw/x86/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../../build/win/mingw/x86/debug/test
        }
        release {
          message(Release)
          DESTDIR = ../../build/win/mingw/x86/release/test
        }
      }
    }

    win32-msvc2015 {
      contains(QT_ARCH, x86_64) {
        message(Windows x86_64 MSVC)

        SOURCES += ../cryptography/botan/windows/msvc/x86_64/botan_all.cpp
        HEADERS += ../cryptography/botan/windows/msvc/x86_64/botan_all.h \
                   ../cryptography/botan/windows/msvc/x86_64/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../../build/win/msvc/x86_64/debug/test
        }
        release {
          message(Release)
          DESTDIR = ../../build/win/msvc/x86_64/release/test
        }
      } else {
        message(Windows x86 MSVC)
        SOURCES += ../cryptography/botan/windows/msvc/x86/botan_all.cpp
        HEADERS += ../cryptography/botan/windows/msvc/x86/botan_all.h \
                   ../cryptography/botan/windows/msvc/x86/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../../build/win/msvc/x86/debug/test
        }
        release {
          message(Release)
          DESTDIR = ../../build/win/msvc/x86/release/test
        }
      }
    }
  } # End win32
} # End desktop

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/qrc
