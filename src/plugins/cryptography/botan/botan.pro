include(../../../../defaults.pri)

QT += core

TARGET = botan

TEMPLATE = lib

CONFIG += plugin static c++14

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

SOURCES += BotanProvider.cpp

HEADERS += \
  BotanProvider.hpp \
  $$PWD/../../../core/Constants.hpp \
  $$PWD/../../../core/SchedulerState.hpp

OTHER_FILES += botan.json

# Platform-specific configuration
linux {
  message(Linux)

  android {
    message(Android)

    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
      message(armeabi-v7a)

      SOURCES += botan/android/armv7/botan_all.cpp

      HEADERS += \
        botan/android/armv7/botan_all.h \
        botan/android/armv7/botan_all_internal.h

      debug {
        message(Debug)
        DESTDIR = $$PWD/../../../../build/android/armv7/debug/plugins/cryptography/botan
      }
      release {
        message(Release)
        DESTDIR = $$PWD/../../../../build/android/armv7/release/plugins/cryptography/botan
      }
    }

    contains(ANDROID_TARGET_ARCH, arm64-v8a) {
      message(arm64-v8a)

      SOURCES += botan/android/arm64_v8a/botan_all.cpp

      HEADERS += \
        botan/android/arm64_v8a/botan_all.h \
        botan/android/arm64_v8a/botan_all_internal.h

      debug {
        message(Debug)
        DESTDIR = $$PWD/../../../../build/android/arm64_v8a/debug/plugins/cryptography/botan
      }
      release {
        message(Release)
        DESTDIR = $$PWD/../../../../build/android/arm64_v8a/release/plugins/cryptography/botan
      }
    }
  } # End android

  linux-clang {
    message(clang)

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
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
  } # End linux-clang

  linux-g++-64 {
    message(g++ x86_64)

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
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
  } # End linux-g++-64
} # End linux

darwin {
  LIBS += -framework Security

  ios {
    message(iOS)
    message(clang)

    CONFIG -= simulator

    SOURCES += \
      botan/iOS/arm64/botan_all.cpp \
      botan/iOS/arm64/botan_all_armv8crypto.cpp

    HEADERS += \
      botan/iOS/arm64/botan_all.h \
      botan/iOS/arm64/botan_all_internal.h

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

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector

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
