include(../../../../defaults.pri)

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
  $$PWD/../../../app/Constants.cpp \
  $$PWD/../../../app/DispatcherState.cpp

HEADERS += \
  BotanProvider.hpp \
  $$PWD/../../../app/Constants.hpp \
  $$PWD/../../../app/DispatcherState.hpp

# Platform-specific configuration
linux {
  message(Linux)

  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  android {
    message(Android)

#    SOURCES += botan/android/botan_all.cpp

#    HEADERS += \
#      botan/android/botan_all.h \
#      botan/android/android_to_string.h

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../../../build/android/debug/plugins/cryptography/botan
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/android/release/plugins/cryptography/botan
    }
  } # End android

  linux-clang {
    message(clang)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    SOURCES += \
      botan/linux/clang/x86_64/botan_all.cpp \
      botan/linux/clang/x86_64/botan_all_aesni.cpp \
      botan/linux/clang/x86_64/botan_all_avx2.cpp \
      botan/linux/clang/x86_64/botan_all_bmi2.cpp \
      botan/linux/clang/x86_64/botan_all_rdrand.cpp \
      botan/linux/clang/x86_64/botan_all_rdseed.cpp \
      botan/linux/clang/x86_64/botan_all_sha_sse41_ssse3.cpp \
      botan/linux/clang/x86_64/botan_all_sha_ssse3.cpp \
      botan/linux/clang/x86_64/botan_all_ssse3.cpp

    HEADERS += \
      botan/linux/clang/x86_64/botan_all.h \
      botan/linux/clang/x86_64/botan_all_internal.h

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../../../build/linux/clang/x86_64/debug/plugins/cryptograpy/botan
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/linux/clang/x86_64/release/plugins/cryptography/botan
    }
  } # End clang

  linux-g++-64 {
    message(g++ x86_64)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    SOURCES += \
      botan/linux/gcc/x86_64/botan_all.cpp \
      botan/linux/gcc/x86_64/botan_all_aesni.cpp \
      botan/linux/gcc/x86_64/botan_all_avx2.cpp \
      botan/linux/gcc/x86_64/botan_all_bmi2.cpp \
      botan/linux/gcc/x86_64/botan_all_rdrand.cpp \
      botan/linux/gcc/x86_64/botan_all_rdseed.cpp \
      botan/linux/gcc/x86_64/botan_all_sha_sse41_ssse3.cpp \
      botan/linux/gcc/x86_64/botan_all_sha_ssse3.cpp \
      botan/linux/gcc/x86_64/botan_all_ssse3.cpp

    HEADERS += \
      botan/linux/gcc/x86_64/botan_all.h \
      botan/linux/gcc/x86_64/botan_all_internal.h

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../../../build/linux/gcc/x86_64/debug/plugins/cryptography/botan
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/linux/gcc/x86_64/release/plugins/cryptography/botan
    }
  } # End g++ x86_64
} # End linux

darwin {
  QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
  QMAKE_LFLAGS += -fstack-protector

  LIBS += -framework Security

  ios {
    message(iOS)
    message(clang)

#    SOURCES += botan/iOS/botan_all.cpp

#    HEADERS += botan/iOS/botan_all.h

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../../../build/iOS/debug/plugins/cryptography/botan
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/iOS/release/plugins/cryptography/botan
    }
  } # End ios

  macos {
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
      DESTDIR = $$PWD/../../../../build/macOS/clang/x86_64/debug/plugins/cryptography/botan
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/macOS/clang/x86_64/release/plugins/cryptography/botan
    }
  } # End macos
} # End darwin

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
      DESTDIR = $$PWD/../../../../build/windows/mingw/x86_32/debug/plugins/cryptography/botan
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../../../build/windows/mingw/x86_32/release/plugins/cryptography/botan
    }
  } # End win32-g++

  win32-msvc {
    message(MSVC)

    LIBS += -ladvapi32 -luser32 -lws2_32

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
        DESTDIR = $$PWD/../../../../build/windows/msvc/x86_64/debug/plugins/cryptography/botan
      }
      release {
        message(Release)
        DESTDIR = $$PWD/../../../../build/windows/msvc/x86_64/release/plugins/cryptography/botan
      }
    }
  } # End win32-msvc
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
