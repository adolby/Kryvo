TARGET = botan

TEMPLATE = lib

CONFIG += c++14 plugin

QT += core

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
  ../../app/cryptography/CryptoState.cpp

HEADERS += \
  BotanProvider.hpp \
  ../../app/cryptography/CryptoState.hpp

INCLUDEPATH = ../../app/

# Platform-specific configuration
android|ios {
  message(Mobile)

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
      DESTDIR = ../../../build/android/debug/plugins/botan
    }
    release {
      message(Release)
      DESTDIR = ../../../build/android/release/plugins/botan
    }
  }

  ios {
    message(iOS)
    message(clang)

    SOURCES += \
      botan/iOS/botan_all.cpp

    HEADERS += \
      botan/iOS/botan_all.h

    debug {
      message(Debug)
      DESTDIR = ../../../build/iOS/debug/plugins/botan
    }
    release {
      message(Release)
      DESTDIR = ../../../build/iOS/release/plugins/botan
    }
  }

  OBJECTS_DIR = $${DESTDIR}/obj
  MOC_DIR = $${DESTDIR}/moc
} else { # Desktop OS
  message(Desktop)

  linux {
    message(Linux)

    LIBS += -lz
    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    linux-clang {
      message(clang x86_64)

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
        DESTDIR = ../../../build/linux/clang/x86_64/debug/plugins/botan
      }
      release {
        message(Release)
        DESTDIR = ../../../build/linux/clang/x86_64/release/plugins/botan
      }
    }

    linux-g++-64 {
      message(g++ x86_64)

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
        DESTDIR = ../../../build/linux/gcc/x86_64/debug/plugins/botan
      }
      release {
        message(Release)
        DESTDIR = ../../../build/linux/gcc/x86_64/release/plugins/botan
      }
    }

    OBJECTS_DIR = $${DESTDIR}/obj
    MOC_DIR = $${DESTDIR}/moc

    mkpath($${DESTDIR}/../../Kryvo/plugins/)
    mkpath($${DESTDIR}/../../test/plugins/)
    QMAKE_POST_LINK += $$quote($$QMAKE_COPY $${DESTDIR}/libbotan.so $${DESTDIR}/../../Kryvo/plugins/)
    QMAKE_POST_LINK += $$quote(&& $$QMAKE_COPY $${DESTDIR}/libbotan.so $${DESTDIR}/../../test/plugins/)
  } # End Linux

  macx {
    message(macOS)
    message(clang x86_64)

    QMAKE_MAC_SDK = macosx10.12

    LIBS += -lz
    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector

    SOURCES += \
      botan/macOS/clang/x86_64/botan_all.cpp \
      botan/macOS/clang/x86_64/botan_all_aesni.cpp \
      botan/macOS/clang/x86_64/botan_all_avx2.cpp \
      botan/macOS/clang/x86_64/botan_all_rdrand.cpp \
      botan/macOS/clang/x86_64/botan_all_rdseed.cpp \
      botan/macOS/clang/x86_64/botan_all_ssse3.cpp

    HEADERS += \
      botan/macOS/clang/x86_64/botan_all.h \
      botan/macOS/clang/x86_64/botan_all_internal.h

    debug {
      message(Debug)
      DESTDIR = ../../../build/macOS/clang/x86_64/debug/plugins/botan
    }
    release {
      message(Release)
      DESTDIR = ../../../build/macOS/clang/x86_64/release/plugins/botan
    }

    OBJECTS_DIR = $${DESTDIR}/obj
    MOC_DIR = $${DESTDIR}/moc

    mkpath($${DESTDIR}/../../Kryvo/Kryvo.app/Contents/plugins/)
    mkpath($${DESTDIR}/../../test/CryptoTests.app/Contents/plugins/)
    QMAKE_POST_LINK += $$quote($$QMAKE_COPY $${DESTDIR}/libbotan.dylib $${DESTDIR}/../../Kryvo/Kryvo.app/Contents/plugins/)
    QMAKE_POST_LINK += $$quote(&& $$QMAKE_COPY $${DESTDIR}/libbotan.dylib $${DESTDIR}/../../test/CryptoTests.app/Contents/plugins/)
  } # End macOS

  win32 {
    message(Windows)

    win32-msvc2015 {
      LIBS += advapi32.lib user32.lib

      QMAKE_CXXFLAGS += -bigobj -arch:AVX2

      contains(QT_ARCH, x86_64) {
        message(MSVC x86_64)

        SOURCES += \
          botan/windows/msvc/x86_64/botan_all.cpp \
          botan/windows/msvc/x86_64/botan_all_aesni.cpp \
          botan/windows/msvc/x86_64/botan_all_avx2.cpp \
          botan/windows/msvc/x86_64/botan_all_rdrand.cpp \
          botan/windows/msvc/x86_64/botan_all_rdseed.cpp \
          botan/windows/msvc/x86_64/botan_all_ssse3.cpp

        HEADERS += \
          botan/windows/msvc/x86_64/botan_all.h \
          botan/windows/msvc/x86_64/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../../../build/windows/msvc/x86_64/debug/plugins/botan
        }
        release {
          message(Release)
          DESTDIR = ../../../build/windows/msvc/x86_64/release/plugins/botan
        }
      } else {
        message(MSVC x86)

        SOURCES += \
          botan/windows/msvc/x86/botan_all.cpp \
          botan/windows/msvc/x86/botan_all_aesni.cpp \
          botan/windows/msvc/x86/botan_all_avx2.cpp \
          botan/windows/msvc/x86/botan_all_rdrand.cpp \
          botan/windows/msvc/x86/botan_all_rdseed.cpp \
          botan/windows/msvc/x86/botan_all_ssse3.cpp

        HEADERS += \
          botan/windows/msvc/x86/botan_all.h \
          botan/windows/msvc/x86/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../../../build/windows/msvc/x86/debug/plugins/botan
        }
        release {
          message(Release)
          DESTDIR = ../../../build/windows/msvc/x86/release/plugins/botan
        }
      }
    }

    OBJECTS_DIR = $${DESTDIR}/obj
    MOC_DIR = $${DESTDIR}/moc

    mkpath($${DESTDIR}/../../Kryvo/plugins/)
    mkpath($${DESTDIR}/../../test/plugins/)
    QMAKE_POST_LINK += $$quote($$QMAKE_COPY $${DESTDIR}/libbotan.dll $${DESTDIR}/../Kryvo/plugins/)
    QMAKE_POST_LINK += $$quote($$QMAKE_COPY $${DESTDIR}/libbotan.dll $${DESTDIR}/../test/plugins/)
  } # End win32
} # End desktop
