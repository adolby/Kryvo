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
  src/Application.cpp \
  src/archive/Archiver.cpp \
  src/cryptography/Crypto.cpp \
  src/cryptography/State.cpp \
  src/cryptography/BotanCrypto.cpp \
  src/gui/MainWindow.cpp \
  src/gui/SettingsFrame.cpp \
  src/gui/HeaderFrame.cpp \
  src/gui/FileListFrame.cpp \
  src/gui/FileListDelegate.cpp \
  src/gui/ProgressFrame.cpp \
  src/gui/MessageFrame.cpp \
  src/gui/OutputFrame.cpp \
  src/gui/PasswordFrame.cpp \
  src/gui/ControlButtonFrame.cpp \
  src/gui/ElidedLabel.cpp \
  src/gui/flowlayout.cpp \
  src/gui/SlidingStackedWidget.cpp \
  src/settings/Settings.cpp \
  src/utility/Thread.cpp

HEADERS += \
  src/constants.h \
  src/Application.hpp \
  src/archive/Archiver.hpp \
  src/cryptography/Crypto.hpp \
  src/cryptography/State.hpp \
  src/cryptography/BotanCrypto.hpp \
  src/gui/MainWindow.hpp \
  src/gui/SettingsFrame.hpp \
  src/gui/HeaderFrame.hpp \
  src/gui/FileListFrame.hpp \
  src/gui/FileListDelegate.hpp \
  src/gui/ProgressFrame.hpp \
  src/gui/MessageFrame.hpp \
  src/gui/OutputFrame.hpp \
  src/gui/PasswordFrame.hpp \
  src/gui/ControlButtonFrame.hpp \
  src/gui/ElidedLabel.hpp \
  src/gui/flowlayout.h \
  src/gui/SlidingStackedWidget.hpp \
  src/settings/Settings.hpp \
  src/utility/Thread.hpp \
  src/utility/pimpl_impl.h \
  src/utility/pimpl.h

# Include QuaZip files
DEFINES += QUAZIP_STATIC
include(src/libs/quazip/quazip.pri)

# Platform-specific configuration
android {
  message(Android)

  # You'll need to place your Boost path here
  INCLUDEPATH += $$(HOME)/Boost/boost_1_58_0/

  SOURCES += \
    # src/cryptography/botan/android/botan_all.cpp \
    src/gui/TouchMainWindow.cpp

  HEADERS += \
    # src/cryptography/botan/android/botan_all.h \
    # src/cryptography/botan/android/android_to_string.h \
    src/gui/TouchMainWindow.hpp

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

    QMAKE_TARGET_BUNDLE_PREFIX = com.andrewdolby
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
      LIBS += advapi32.lib user32.lib

      INCLUDEPATH += $$PWD/src/libs/zlib/

      QMAKE_CXXFLAGS += -bigobj -arch:AVX2

      SOURCES += \
        src/libs/zlib/adler32.c \
        src/libs/zlib/compress.c \
        src/libs/zlib/crc32.c \
        src/libs/zlib/deflate.c \
        src/libs/zlib/gzclose.c \
        src/libs/zlib/gzlib.c \
        src/libs/zlib/gzread.c \
        src/libs/zlib/gzwrite.c \
        src/libs/zlib/infback.c \
        src/libs/zlib/inffast.c \
        src/libs/zlib/inflate.c \
        src/libs/zlib/inftrees.c \
        src/libs/zlib/trees.c \
        src/libs/zlib/uncompr.c \
        src/libs/zlib/zutil.c

      HEADERS += \
        src/libs/zlib/zlib.h \
        src/libs/zlib/crc32.h \
        src/libs/zlib/deflate.h \
        src/libs/zlib/gzguts.h \
        src/libs/zlib/inffast.h \
        src/libs/zlib/inffixed.h \
        src/libs/zlib/inflate.h \
        src/libs/zlib/inftrees.h \
        src/libs/zlib/trees.h \
        src/libs/zlib/zconf.h \
        src/libs/zlib/zutil.h

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

RESOURCES += \
  resources/assets.qrc
