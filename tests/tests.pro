QT += core testlib
QT -= gui

TARGET = CryptoTests

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

INCLUDEPATH += ..

SOURCES += \
  ../src/cryptography/Crypto.cpp \
  src/test_Crypto.cpp

HEADERS += \
  ../src/cryptography/Crypto.hpp \
  src/test_Crypto.hpp

# Platform-specific configuration
android-g++ {
  message(Android)
  message(g++)

  # You'll need to place your Boost path here.
  INCLUDEPATH += $$(HOME)/Boost/boost_1_58_0/

  SOURCES += ../src/cryptography/botan/android/botan_all.cpp

  HEADERS += \
    ../src/cryptography/botan/android/botan_all.h \
    ../src/cryptography/botan/android/android_to_string.h

  ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

  OTHER_FILES += ../android/AndroidManifest.xml

  debug {
    message(Debug)
    DESTDIR = ../build/android/debug/test/
  }
  release {
    message(Release)
    DESTDIR = ../build/android/release/test/
  }
} else:ios {
  message(iOS)
  message(clang)

  SOURCES += ../src/cryptography/botan/iOS/botan_all.cpp

  HEADERS += ../src/cryptography/botan/iOS/botan_all.h

  debug {
    message(Debug)
    DESTDIR = ../build/iOS/debug/test/
  }
  release {
    message(Release)
    DESTDIR = ../build/iOS/release/test/
  }
} else { # Desktop OS
  linux {
    message(Linux)

    LIBS += -lz
    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    linux-clang {
      message(clang x86_64)

      SOURCES += \
        ../src/cryptography/botan/linux/clang/x86_64/botan_all.cpp \
        ../src/cryptography/botan/linux/clang/x86_64/botan_all_aesni.cpp \
        ../src/cryptography/botan/linux/clang/x86_64/botan_all_avx2.cpp \
        ../src/cryptography/botan/linux/clang/x86_64/botan_all_rdrand.cpp \
        ../src/cryptography/botan/linux/clang/x86_64/botan_all_rdseed.cpp \
        ../src/cryptography/botan/linux/clang/x86_64/botan_all_ssse3.cpp

      HEADERS += \
        ../src/cryptography/botan/linux/clang/x86_64/botan_all.h \
        ../src/cryptography/botan/linux/clang/x86_64/botan_all_internal.h

      debug {
        message(Debug)
        DESTDIR = ../build/linux/clang/x86_64/debug/test/
      }
      release {
        message(Release)
        DESTDIR = ../build/linux/clang/x86_64/release/test/
      }
    }

    linux-g++-64 {
      message(g++ x86_64)

      SOURCES += \
        ../src/cryptography/botan/linux/gcc/x86_64/botan_all.cpp \
        ../src/cryptography/botan/linux/gcc/x86_64/botan_all_aesni.cpp \
        ../src/cryptography/botan/linux/gcc/x86_64/botan_all_avx2.cpp \
        ../src/cryptography/botan/linux/gcc/x86_64/botan_all_rdrand.cpp \
        ../src/cryptography/botan/linux/gcc/x86_64/botan_all_rdseed.cpp \
        ../src/cryptography/botan/linux/gcc/x86_64/botan_all_ssse3.cpp

      HEADERS += \
        ../src/cryptography/botan/linux/gcc/x86_64/botan_all.h \
        ../src/cryptography/botan/linux/gcc/x86_64/botan_all_internal.h

      debug {
        message(Debug)
        DESTDIR = ../build/linux/gcc/x86_64/debug/test/
      }
      release {
        message(Release)
        DESTDIR = ../build/linux/gcc/x86_64/release/test/
      }
    }
  } # End Linux

  macx {
    message(macOS)
    message(clang x86_64)

    LIBS += -lz
    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector

    SOURCES += \
      ../src/cryptography/botan/macOS/clang/x86_64/botan_all.cpp \
      ../src/cryptography/botan/macOS/clang/x86_64/botan_all_aesni.cpp \
      ../src/cryptography/botan/macOS/clang/x86_64/botan_all_avx2.cpp \
      ../src/cryptography/botan/macOS/clang/x86_64/botan_all_rdrand.cpp \
      ../src/cryptography/botan/macOS/clang/x86_64/botan_all_rdseed.cpp \
      ../src/cryptography/botan/macOS/clang/x86_64/botan_all_ssse3.cpp

    HEADERS += \
      ../src/cryptography/botan/macOS/clang/x86_64/botan_all.h \
      ../src/cryptography/botan/macOS/clang/x86_64/botan_all_internal.h

    debug {
      message(Debug)
      DESTDIR = ../build/macOS/clang/x86_64/debug/test/
    }
    release {
      message(Release)
      DESTDIR = ../build/macOS/clang/x86_64/release/test/
    }
  }

  win32 {
    message(Windows)

    win32-msvc2015 {
      QMAKE_CXXFLAGS += -bigobj -arch:AVX2

      LIBS += advapi32.lib user32.lib

      INCLUDEPATH += $$PWD/src/cryptography/botan/zlib/

      SOURCES += \
        ../src/cryptography/botan/zlib/adler32.c \
        ../src/cryptography/botan/zlib/compress.c \
        ../src/cryptography/botan/zlib/crc32.c \
        ../src/cryptography/botan/zlib/deflate.c \
        ../src/cryptography/botan/zlib/gzclose.c \
        ../src/cryptography/botan/zlib/gzlib.c \
        ../src/cryptography/botan/zlib/gzread.c \
        ../src/cryptography/botan/zlib/gzwrite.c \
        ../src/cryptography/botan/zlib/infback.c \
        ../src/cryptography/botan/zlib/inffast.c \
        ../src/cryptography/botan/zlib/inflate.c \
        ../src/cryptography/botan/zlib/inftrees.c \
        ../src/cryptography/botan/zlib/trees.c \
        ../src/cryptography/botan/zlib/uncompr.c \
        ../src/cryptography/botan/zlib/zutil.c

      HEADERS += \
        ../src/cryptography/botan/zlib/zlib.h \
        ../src/cryptography/botan/zlib/crc32.h \
        ../src/cryptography/botan/zlib/deflate.h \
        ../src/cryptography/botan/zlib/gzguts.h \
        ../src/cryptography/botan/zlib/inffast.h \
        ../src/cryptography/botan/zlib/inffixed.h \
        ../src/cryptography/botan/zlib/inflate.h \
        ../src/cryptography/botan/zlib/inftrees.h \
        ../src/cryptography/botan/zlib/trees.h \
        ../src/cryptography/botan/zlib/zconf.h \
        ../src/cryptography/botan/zlib/zutil.h

      contains(QT_ARCH, x86_64) {
        message(MSVC x86_64)

        SOURCES += \
          ../src/cryptography/botan/windows/msvc/x86_64/botan_all.cpp \
          ../src/cryptography/botan/windows/msvc/x86_64/botan_all_aesni.cpp \
          ../src/cryptography/botan/windows/msvc/x86_64/botan_all_avx2.cpp \
          ../src/cryptography/botan/windows/msvc/x86_64/botan_all_rdrand.cpp \
          ../src/cryptography/botan/windows/msvc/x86_64/botan_all_rdseed.cpp \
          ../src/cryptography/botan/windows/msvc/x86_64/botan_all_ssse3.cpp

        HEADERS += \
          ../src/cryptography/botan/windows/msvc/x86_64/botan_all.h \
          ../src/cryptography/botan/windows/msvc/x86_64/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../build/windows/msvc/x86_64/debug/test/
        }
        release {
          message(Release)
          DESTDIR = ../build/windows/msvc/x86_64/release/test/
        }
      } else {
        message(MSVC x86)

        SOURCES += \
          ../src/cryptography/botan/windows/msvc/x86/botan_all.cpp \
          ../src/cryptography/botan/windows/msvc/x86/botan_all_aesni.cpp \
          ../src/cryptography/botan/windows/msvc/x86/botan_all_avx2.cpp \
          ../src/cryptography/botan/windows/msvc/x86/botan_all_rdrand.cpp \
          ../src/cryptography/botan/windows/msvc/x86/botan_all_rdseed.cpp \
          ../src/cryptography/botan/windows/msvc/x86/botan_all_ssse3.cpp

        HEADERS += \
          ../src/cryptography/botan/windows/msvc/x86/botan_all.h \
          ../src/cryptography/botan/windows/msvc/x86/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../build/windows/msvc/x86/debug/test/
        }
        release {
          message(Release)
          DESTDIR = ../build/windows/msvc/x86/release/test/
        }
      }
    }
  } # End win32
} # End desktop

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/qrc
