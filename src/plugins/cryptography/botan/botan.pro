QT += core

TARGET = botan

TEMPLATE = lib

CONFIG += c++14 plugin

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
  BotanProvider.cpp \
  ../../../app/Constants.cpp \
  ../../../app/cryptography/CryptoState.cpp \
  zlib/adler32.c \
  zlib/compress.c \
  zlib/crc32.c \
  zlib/deflate.c \
  zlib/gzclose.c \
  zlib/gzlib.c \
  zlib/gzread.c \
  zlib/gzwrite.c \
  zlib/infback.c \
  zlib/inffast.c \
  zlib/inflate.c \
  zlib/inftrees.c \
  zlib/trees.c \
  zlib/uncompr.c \
  zlib/zutil.c

HEADERS += \
  BotanProvider.hpp \
  ../../../app/Constants.hpp \
  ../../../app/cryptography/CryptoState.hpp \
  zlib/crc32.h \
  zlib/deflate.h \
  zlib/gzguts.h \
  zlib/inffast.h \
  zlib/inffixed.h \
  zlib/inflate.h \
  zlib/inftrees.h \
  zlib/trees.h \
  zlib/zconf.h \
  zlib/zlib.h \
  zlib/zutil.h

INCLUDEPATH += \
  ../../../app/ \
  zlib/

# Platform-specific configuration
linux {
  message(Linux)

  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  android {
    message(Android)

    # Place Boost path here
    INCLUDEPATH += $$(HOME)/Boost/boost_1_58_0/

    SOURCES += botan/android/botan_all.cpp

    HEADERS += \
      botan/android/botan_all.h \
      botan/android/android_to_string.h

    debug {
      message(Debug)
      DESTDIR = ../../../../build/android/debug/plugins/botan
    }
    release {
      message(Release)
      DESTDIR = ../../../../build/android/release/plugins/botan
    }
  } # End Android

  linux-clang {
    message(clang)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    SOURCES += \
      botan/linux/clang/x86_64/botan_all.cpp \
      botan/linux/clang/x86_64/botan_all_aesni.cpp \
      botan/linux/clang/x86_64/botan_all_avx2.cpp \
      botan/linux/clang/x86_64/botan_all_rdrand.cpp \
      botan/linux/clang/x86_64/botan_all_rdseed.cpp \
      botan/linux/clang/x86_64/botan_all_ssse3.cpp

    HEADERS += \
      botan/linux/clang/x86_64/botan_all.h \
      botan/linux/clang/x86_64/botan_all_internal.h

    debug {
      message(Debug)
      DESTDIR = ../../../../build/linux/clang/x86_64/debug/plugins/botan
    }
    release {
      message(Release)
      DESTDIR = ../../../../build/linux/clang/x86_64/release/plugins/botan
    }
  } # End clang

  linux-g++ {
    message(g++)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    SOURCES += \
      botan/linux/gcc/x86_64/botan_all.cpp \
      botan/linux/gcc/x86_64/botan_all_aesni.cpp \
      botan/linux/gcc/x86_64/botan_all_avx2.cpp \
      botan/linux/gcc/x86_64/botan_all_rdrand.cpp \
      botan/linux/gcc/x86_64/botan_all_rdseed.cpp \
      botan/linux/gcc/x86_64/botan_all_ssse3.cpp

    HEADERS += \
      botan/linux/gcc/x86_64/botan_all.h \
      botan/linux/gcc/x86_64/botan_all_internal.h

    debug {
      message(Debug)
      DESTDIR = ../../../../build/linux/gcc/x86_64/debug/plugins/botan
    }
    release {
      message(Release)
      DESTDIR = ../../../../build/linux/gcc/x86_64/release/plugins/botan
    }
  } # End g++
} # End Linux

mac {
  QMAKE_MAC_SDK = macosx10.13
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  LIBS += -framework Security

  ios {
    message(iOS)
    message(clang)

    SOURCES += botan/iOS/botan_all.cpp

    HEADERS += botan/iOS/botan_all.h

    debug {
      message(Debug)
      DESTDIR = ../../../../build/iOS/debug/plugins/botan
    }
    release {
      message(Release)
      DESTDIR = ../../../../build/iOS/release/plugins/botan
    }
  } # End iOS

  macx {
    message(macOS)
    message(clang)

    SOURCES += \
      botan/macOS/clang/x86_64/botan_all.cpp \
      botan/macOS/clang/x86_64/botan_all_aesni.cpp \
      botan/macOS/clang/x86_64/botan_all_avx2.cpp \
      botan/macOS/clang/x86_64/botan_all_bmi2.cpp \
      botan/macOS/clang/x86_64/botan_all_rdrand.cpp \
      botan/macOS/clang/x86_64/botan_all_rdseed.cpp \
      botan/macOS/clang/x86_64/botan_all_sha_sse41_ssse3.cpp \
      botan/macOS/clang/x86_64/botan_all_sha_ssse3.cpp \
      botan/macOS/clang/x86_64/botan_all_ssse3.cpp

    HEADERS += \
      botan/macOS/clang/x86_64/botan_all.h \
      botan/macOS/clang/x86_64/botan_all_internal.h

    debug {
      message(Debug)
      DESTDIR = ../../../../build/macOS/clang/x86_64/debug/plugins/botan
    }
    release {
      message(Release)
      DESTDIR = ../../../../build/macOS/clang/x86_64/release/plugins/botan
    }
  } # End macOS
} # End Mac

win32 {
  message(Windows)

  win32-g++ {
    message(g++)

    SOURCES += \
      botan/windows/mingw/x86_32/botan_all.cpp \
      botan/windows/mingw/x86_32/botan_all_aesni.cpp \
      botan/windows/mingw/x86_32/botan_all_avx2.cpp \
      botan/windows/mingw/x86_32/botan_all_bmi2.cpp \
      botan/windows/mingw/x86_32/botan_all_rdrand.cpp \
      botan/windows/mingw/x86_32/botan_all_rdseed.cpp \
      botan/windows/mingw/x86_32/botan_all_sha_sse41_ssse3.cpp \
      botan/windows/mingw/x86_32/botan_all_sha_ssse3.cpp \
      botan/windows/mingw/x86_32/botan_all_sse2.cpp \
      botan/windows/mingw/x86_32/botan_all_ssse3.cpp

    HEADERS += \
      botan/windows/mingw/x86_32/botan_all.h \
      botan/windows/mingw/x86_32/botan_all_internal.h

    debug {
      message(Debug)
      DESTDIR = ../../build/windows/mingw/x86_32/debug/plugins/botan/
    }
    release {
      message(Release)
      DESTDIR = ../../build/windows/mingw/x86_32/release/plugins/botan/
    }
  }

  win32-msvc {
    message(MSVC)

    LIBS += advapi32.lib user32.lib ws2_32.lib

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(x86_64)

      SOURCES += \
        botan/windows/msvc/x86_64/botan_all.cpp \
        botan/windows/msvc/x86_64/botan_all_aesni.cpp \
        botan/windows/msvc/x86_64/botan_all_avx2.cpp \
        botan/windows/msvc/x86_64/botan_all_rdrand.cpp \
        botan/windows/msvc/x86_64/botan_all_rdseed.cpp \
        botan/windows/msvc/x86_64/botan_all_sha_sse41_ssse3.cpp \
        botan/windows/msvc/x86_64/botan_all_sha_ssse3.cpp \
        botan/windows/msvc/x86_64/botan_all_ssse3.cpp

      HEADERS += \
        botan/windows/msvc/x86_64/botan_all.h \
        botan/windows/msvc/x86_64/botan_all_internal.h

      debug {
        message(Debug)
        DESTDIR = ../../../../build/windows/msvc/x86_64/debug/plugins/botan
      }
      release {
        message(Release)
        DESTDIR = ../../../../build/windows/msvc/x86_64/release/plugins/botan
      }
    }
  }
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc