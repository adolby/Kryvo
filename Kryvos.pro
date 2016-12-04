QT += core gui widgets

TARGET = Kryvos

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

SOURCES += \
  src/main.cpp \
  src/Kryvos.cpp \
  src/cryptography/Crypto.cpp \
  src/gui/MainWindow.cpp \
  src/gui/SettingsFrame.cpp \
  src/gui/HeaderFrame.cpp \
  src/gui/FileListFrame.cpp \
  src/gui/FileListDelegate.cpp \
  src/gui/MessageFrame.cpp \
  src/gui/PasswordFrame.cpp \
  src/gui/ControlButtonFrame.cpp \
  src/gui/SlideSwitch.cpp \
  src/gui/flowlayout.cpp \
  src/gui/FluidLayout.cpp \
  src/gui/SlidingStackedWidget.cpp \
  src/settings/Settings.cpp

HEADERS += \
  src/Kryvos.hpp \
  src/cryptography/Crypto.hpp \
  src/gui/MainWindow.hpp \
  src/gui/SettingsFrame.hpp \
  src/gui/HeaderFrame.hpp \
  src/gui/FileListFrame.hpp \
  src/gui/FileListDelegate.hpp \
  src/gui/MessageFrame.hpp \
  src/gui/PasswordFrame.hpp \
  src/gui/ControlButtonFrame.hpp \
  src/gui/SlideSwitch.hpp \
  src/gui/flowlayout.h \
  src/gui/FluidLayout.hpp \
  src/gui/SlidingStackedWidget.hpp \
  src/settings/Settings.hpp \
  src/utility/pimpl_impl.h \
  src/utility/pimpl.h

# Platform-specific configuration
android {
  message(Android)

  # You'll need to place your Boost path here.
  INCLUDEPATH += $$(HOME)/Boost/boost_1_58_0/

  SOURCES += \
    # src/cryptography/botan/android/botan_all.cpp \
    src/gui/TouchMainWindow.cpp

  HEADERS += \
    # src/cryptography/botan/android/botan_all.h \
    # src/cryptography/botan/android/android_to_string.h \
    src/gui/TouchMainWindow.hpp

  ANDROID_PACKAGE_SOURCE_DIR = resources/android/

  OTHER_FILES += resources/android/AndroidManifest.xml

  debug {
    message(Debug)
    DESTDIR = build/android/debug/
  }
  release {
    message(Release)
    DESTDIR = build/android/release/
  }
} else:ios {
  message(iOS)
  message(clang)

  SOURCES += \
    src/cryptography/botan/iOS/botan_all.cpp \
    src/gui/TouchMainWindow.cpp

  HEADERS += \
    src/cryptography/botan/iOS/botan_all.h \
    src/gui/TouchMainWindow.hpp

  debug {
    message(Debug)
    DESTDIR = build/iOS/debug/
  }
  release {
    message(Release)
    DESTDIR = build/iOS/release/
  }
} else { # Desktop OS
  SOURCES += src/gui/DesktopMainWindow.cpp
  HEADERS += src/gui/DesktopMainWindow.hpp

  linux {
    message(Linux)

    LIBS += -lz
    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    linux-clang {
      message(clang x86_64)

      SOURCES += \
        src/cryptography/botan/linux/clang/x86_64/botan_all.cpp \
        src/cryptography/botan/linux/clang/x86_64/botan_all_aesni.cpp \
        src/cryptography/botan/linux/clang/x86_64/botan_all_avx2.cpp \
        src/cryptography/botan/linux/clang/x86_64/botan_all_rdrand.cpp \
        src/cryptography/botan/linux/clang/x86_64/botan_all_rdseed.cpp \
        src/cryptography/botan/linux/clang/x86_64/botan_all_ssse3.cpp

      HEADERS += \
        src/cryptography/botan/linux/clang/x86_64/botan_all.h \
        src/cryptography/botan/linux/clang/x86_64/botan_all_internal.h

      debug {
        message(Debug)
        DESTDIR = build/linux/clang/x86_64/debug/Kryvos/
      }
      release {
        message(Release)
        DESTDIR = build/linux/clang/x86_64/release/Kryvos/
      }
    }

    linux-g++-64 {
      message(g++ x86_64)

      SOURCES += \
        src/cryptography/botan/linux/gcc/x86_64/botan_all.cpp \
        src/cryptography/botan/linux/gcc/x86_64/botan_all_aesni.cpp \
        src/cryptography/botan/linux/gcc/x86_64/botan_all_avx2.cpp \
        src/cryptography/botan/linux/gcc/x86_64/botan_all_rdrand.cpp \
        src/cryptography/botan/linux/gcc/x86_64/botan_all_rdseed.cpp \
        src/cryptography/botan/linux/gcc/x86_64/botan_all_ssse3.cpp

      HEADERS += \
        src/cryptography/botan/linux/gcc/x86_64/botan_all.h \
        src/cryptography/botan/linux/gcc/x86_64/botan_all_internal.h

      debug {
        message(Debug)
        DESTDIR = build/linux/gcc/x86_64/debug/Kryvos/
      }
      release {
        message(Release)
        DESTDIR = build/linux/gcc/x86_64/release/Kryvos/
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
      src/cryptography/botan/macOS/clang/x86_64/botan_all.cpp \
      src/cryptography/botan/macOS/clang/x86_64/botan_all_aesni.cpp \
      src/cryptography/botan/macOS/clang/x86_64/botan_all_avx2.cpp \
      src/cryptography/botan/macOS/clang/x86_64/botan_all_rdrand.cpp \
      src/cryptography/botan/macOS/clang/x86_64/botan_all_rdseed.cpp \
      src/cryptography/botan/macOS/clang/x86_64/botan_all_ssse3.cpp

    HEADERS += \
      src/cryptography/botan/macOS/clang/x86_64/botan_all.h \
      src/cryptography/botan/macOS/clang/x86_64/botan_all_internal.h

    ICON = resources/icons/kryvos.icns

    debug {
      message(Debug)
      DESTDIR = build/macOS/clang/x86_64/debug/
    }
    release {
      message(Release)
      DESTDIR = build/macOS/clang/x86_64/release/
    }
  }

  win32 {
    message(Windows)

    win32-msvc2015 {
      QMAKE_CXXFLAGS += -bigobj -arch:AVX2

      LIBS += advapi32.lib user32.lib

      INCLUDEPATH += $$PWD/src/cryptography/botan/zlib/

      SOURCES += \
        src/cryptography/botan/zlib/adler32.c \
        src/cryptography/botan/zlib/compress.c \
        src/cryptography/botan/zlib/crc32.c \
        src/cryptography/botan/zlib/deflate.c \
        src/cryptography/botan/zlib/gzclose.c \
        src/cryptography/botan/zlib/gzlib.c \
        src/cryptography/botan/zlib/gzread.c \
        src/cryptography/botan/zlib/gzwrite.c \
        src/cryptography/botan/zlib/infback.c \
        src/cryptography/botan/zlib/inffast.c \
        src/cryptography/botan/zlib/inflate.c \
        src/cryptography/botan/zlib/inftrees.c \
        src/cryptography/botan/zlib/trees.c \
        src/cryptography/botan/zlib/uncompr.c \
        src/cryptography/botan/zlib/zutil.c

      HEADERS += \
        src/cryptography/botan/zlib/zlib.h \
        src/cryptography/botan/zlib/crc32.h \
        src/cryptography/botan/zlib/deflate.h \
        src/cryptography/botan/zlib/gzguts.h \
        src/cryptography/botan/zlib/inffast.h \
        src/cryptography/botan/zlib/inffixed.h \
        src/cryptography/botan/zlib/inflate.h \
        src/cryptography/botan/zlib/inftrees.h \
        src/cryptography/botan/zlib/trees.h \
        src/cryptography/botan/zlib/zconf.h \
        src/cryptography/botan/zlib/zutil.h

      contains(QT_ARCH, x86_64) {
        message(MSVC x86_64)

        SOURCES += \
          src/cryptography/botan/windows/msvc/x86_64/botan_all.cpp \
          src/cryptography/botan/windows/msvc/x86_64/botan_all_aesni.cpp \
          src/cryptography/botan/windows/msvc/x86_64/botan_all_avx2.cpp \
          src/cryptography/botan/windows/msvc/x86_64/botan_all_rdrand.cpp \
          src/cryptography/botan/windows/msvc/x86_64/botan_all_rdseed.cpp \
          src/cryptography/botan/windows/msvc/x86_64/botan_all_ssse3.cpp

        HEADERS += \
          src/cryptography/botan/windows/msvc/x86_64/botan_all.h \
          src/cryptography/botan/windows/msvc/x86_64/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = build/windows/msvc/x86_64/debug/Kryvos/
        }
        release {
          message(Release)
          DESTDIR = build/windows/msvc/x86_64/release/Kryvos/
        }
      } else {
        message(MSVC x86)

        SOURCES += \
          src/cryptography/botan/windows/msvc/x86/botan_all.cpp \
          src/cryptography/botan/windows/msvc/x86/botan_all_aesni.cpp \
          src/cryptography/botan/windows/msvc/x86/botan_all_avx2.cpp \
          src/cryptography/botan/windows/msvc/x86/botan_all_rdrand.cpp \
          src/cryptography/botan/windows/msvc/x86/botan_all_rdseed.cpp \
          src/cryptography/botan/windows/msvc/x86/botan_all_ssse3.cpp

        HEADERS += \
          src/cryptography/botan/windows/msvc/x86/botan_all.h \
          src/cryptography/botan/windows/msvc/x86/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = build/windows/msvc/x86/debug/Kryvos/
        }
        release {
          message(Release)
          DESTDIR = build/windows/msvc/x86/release/Kryvos/
        }
      }
    }

    RC_ICONS += resources/icons/kryvos.ico
  } # End win32
} # End desktop

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/qrc

RESOURCES += resources/kryvos.qrc
