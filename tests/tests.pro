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
  ../src/cryptography/BotanCrypto.cpp \
  ../src/cryptography/State.cpp \
  ../src/archive/Archiver.cpp \
  src/test_Crypto.cpp

HEADERS += \
  ../src/cryptography/Crypto.hpp \
  ../src/cryptography/BotanCrypto.hpp \
  ../src/cryptography/State.hpp \
  ../src/archive/Archiver.hpp \
  src/test_Crypto.hpp

# Include QuaZip files
DEFINES += QUAZIP_STATIC
include(../src/libs/quazip/quazip.pri)

# Platform-specific configuration
android {
  message(Android)

  # You'll need to place your Boost path here.
  INCLUDEPATH += $$(HOME)/Boost/boost_1_58_0/

  SOURCES += \
    src/libs/botan/android/botan_all.cpp

  HEADERS += \
    src/libs/botan/android/botan_all.h \
    src/libs/botan/android/android_to_string.h

  ANDROID_PACKAGE_SOURCE_DIR = $$PWD/resources/android

  DISTFILES += \
    resources/android/AndroidManifest.xml \
    resources/android/gradle/wrapper/gradle-wrapper.jar \
    resources/android/gradlew \
    resources/android/res/values/libs.xml \
    resources/android/build.gradle \
    resources/android/gradle/wrapper/gradle-wrapper.properties \
    resources/android/gradlew.bat

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

  SOURCES += ../src/libs/botan/iOS/botan_all.cpp

  HEADERS += ../src/libs/botan/iOS/botan_all.h

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
        ../src/libs/botan/linux/clang/x86_64/botan_all.cpp \
        ../src/libs/botan/linux/clang/x86_64/botan_all_aesni.cpp \
        ../src/libs/botan/linux/clang/x86_64/botan_all_avx2.cpp \
        ../src/libs/botan/linux/clang/x86_64/botan_all_rdrand.cpp \
        ../src/libs/botan/linux/clang/x86_64/botan_all_rdseed.cpp \
        ../src/libs/botan/linux/clang/x86_64/botan_all_ssse3.cpp

      HEADERS += \
        ../src/libs/botan/linux/clang/x86_64/botan_all.h \
        ../src/libs/botan/linux/clang/x86_64/botan_all_internal.h

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
        ../src/libs/botan/linux/gcc/x86_64/botan_all.cpp \
        ../src/libs/botan/linux/gcc/x86_64/botan_all_aesni.cpp \
        ../src/libs/botan/linux/gcc/x86_64/botan_all_avx2.cpp \
        ../src/libs/botan/linux/gcc/x86_64/botan_all_rdrand.cpp \
        ../src/libs/botan/linux/gcc/x86_64/botan_all_rdseed.cpp \
        ../src/libs/botan/linux/gcc/x86_64/botan_all_ssse3.cpp

      HEADERS += \
        ../src/libs/botan/linux/gcc/x86_64/botan_all.h \
        ../src/libs/botan/linux/gcc/x86_64/botan_all_internal.h

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

    QMAKE_MAC_SDK = macosx10.12

    LIBS += -lz
    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector

    SOURCES += \
      ../src/libs/botan/macOS/clang/x86_64/botan_all.cpp \
      ../src/libs/botan/macOS/clang/x86_64/botan_all_aesni.cpp \
      ../src/libs/botan/macOS/clang/x86_64/botan_all_avx2.cpp \
      ../src/libs/botan/macOS/clang/x86_64/botan_all_rdrand.cpp \
      ../src/libs/botan/macOS/clang/x86_64/botan_all_rdseed.cpp \
      ../src/libs/botan/macOS/clang/x86_64/botan_all_ssse3.cpp

    HEADERS += \
      ../src/libs/botan/macOS/clang/x86_64/botan_all.h \
      ../src/libs/botan/macOS/clang/x86_64/botan_all_internal.h

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
      LIBS += advapi32.lib user32.lib

      INCLUDEPATH += $$PWD/src/libs/zlib/

      QMAKE_CXXFLAGS += -bigobj -arch:AVX2

      SOURCES += \
        ../src/libs/zlib/adler32.c \
        ../src/libs/zlib/compress.c \
        ../src/libs/zlib/crc32.c \
        ../src/libs/zlib/deflate.c \
        ../src/libs/zlib/gzclose.c \
        ../src/libs/zlib/gzlib.c \
        ../src/libs/zlib/gzread.c \
        ../src/libs/zlib/gzwrite.c \
        ../src/libs/zlib/infback.c \
        ../src/libs/zlib/inffast.c \
        ../src/libs/zlib/inflate.c \
        ../src/libs/zlib/inftrees.c \
        ../src/libs/zlib/trees.c \
        ../src/libs/zlib/uncompr.c \
        ../src/libs/zlib/zutil.c

      HEADERS += \
        ../src/libs/zlib/zlib.h \
        ../src/libs/zlib/crc32.h \
        ../src/libs/zlib/deflate.h \
        ../src/libs/zlib/gzguts.h \
        ../src/libs/zlib/inffast.h \
        ../src/libs/zlib/inffixed.h \
        ../src/libs/zlib/inflate.h \
        ../src/libs/zlib/inftrees.h \
        ../src/libs/zlib/trees.h \
        ../src/libs/zlib/zconf.h \
        ../src/libs/zlib/zutil.h

      contains(QT_ARCH, x86_64) {
        message(MSVC x86_64)

        SOURCES += \
          ../src/libs/botan/windows/msvc/x86_64/botan_all.cpp \
          ../src/libs/botan/windows/msvc/x86_64/botan_all_aesni.cpp \
          ../src/libs/botan/windows/msvc/x86_64/botan_all_avx2.cpp \
          ../src/libs/botan/windows/msvc/x86_64/botan_all_rdrand.cpp \
          ../src/libs/botan/windows/msvc/x86_64/botan_all_rdseed.cpp \
          ../src/libs/botan/windows/msvc/x86_64/botan_all_ssse3.cpp

        HEADERS += \
          ../src/libs/botan/windows/msvc/x86_64/botan_all.h \
          ../src/libs/botan/windows/msvc/x86_64/botan_all_internal.h

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
          ../src/libs/botan/windows/msvc/x86/botan_all.cpp \
          ../src/libs/botan/windows/msvc/x86/botan_all_aesni.cpp \
          ../src/libs/botan/windows/msvc/x86/botan_all_avx2.cpp \
          ../src/libs/botan/windows/msvc/x86/botan_all_rdrand.cpp \
          ../src/libs/botan/windows/msvc/x86/botan_all_rdseed.cpp \
          ../src/libs/botan/windows/msvc/x86/botan_all_ssse3.cpp

        HEADERS += \
          ../src/libs/botan/windows/msvc/x86/botan_all.h \
          ../src/libs/botan/windows/msvc/x86/botan_all_internal.h

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
