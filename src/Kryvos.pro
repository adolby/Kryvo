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
  main.cpp \
  Kryvos.cpp \
  cryptography/Crypto.cpp \
  gui/MainWindow.cpp \
  gui/SettingsFrame.cpp \
  gui/HeaderFrame.cpp \
  gui/FileListFrame.cpp \
  gui/FileListDelegate.cpp \
  gui/MessageFrame.cpp \
  gui/PasswordFrame.cpp \
  gui/ControlButtonFrame.cpp \
  gui/SlideSwitch.cpp \
  gui/flowlayout.cpp \
  gui/FluidLayout.cpp \
  gui/SlidingStackedWidget.cpp \
  settings/Settings.cpp

HEADERS += \
  Kryvos.hpp \
  cryptography/Crypto.hpp \
  gui/MainWindow.hpp \
  gui/SettingsFrame.hpp \
  gui/HeaderFrame.hpp \
  gui/FileListFrame.hpp \
  gui/FileListDelegate.hpp \
  gui/MessageFrame.hpp \
  gui/PasswordFrame.hpp \
  gui/ControlButtonFrame.hpp \
  gui/SlideSwitch.hpp \
  gui/flowlayout.h \
  gui/FluidLayout.hpp \
  gui/SlidingStackedWidget.hpp \
  settings/Settings.hpp \
  utility/pimpl_impl.h \
  utility/pimpl.h

# Platform-specific configuration
android-g++ {
  message(Android G++)

  # You'll need to place your Boost path here.
  INCLUDEPATH += $$(HOME)/Boost/boost_1_58_0/

  SOURCES += \
    cryptography/botan/android/botan_all.cpp \
    gui/TouchMainWindow.cpp
  HEADERS += \
    cryptography/botan/android/botan_all.h \
    cryptography/botan/android/android_to_string.h \
    gui/TouchMainWindow.hpp

  ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

  OTHER_FILES += android/AndroidManifest.xml

  debug {
    message(Debug)
    DESTDIR = ../build/android/debug/
  }
  release {
    message(Release)
    DESTDIR = ../build/android/release/
  }
} else:ios {
  message(iOS)

  SOURCES += \
    cryptography/botan/iOS/botan_all.cpp \
    gui/TouchMainWindow.cpp
  HEADERS += \
    cryptography/botan/iOS/botan_all.h \
    gui/TouchMainWindow.hpp

  debug {
    message(Debug)
    DESTDIR = ../build/iOS/debug/
  }
  release {
    message(Release)
    DESTDIR = ../build/iOS/release/
  }
} else { # Desktop OS
  SOURCES += gui/DesktopMainWindow.cpp
  HEADERS += gui/DesktopMainWindow.hpp

  linux {
    message(Linux)
    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    linux-clang {
      message(Linux clang x86_64)
      SOURCES += \
        cryptography/botan/linux/clang/x86_64/botan_all.cpp \
        cryptography/botan/linux/clang/x86_64/botan_all_aesni.cpp \
        cryptography/botan/linux/clang/x86_64/botan_all_avx2.cpp \
        cryptography/botan/linux/clang/x86_64/botan_all_rdrand.cpp \
        cryptography/botan/linux/clang/x86_64/botan_all_rdseed.cpp \
        cryptography/botan/linux/clang/x86_64/botan_all_ssse3.cpp
      HEADERS += \
        cryptography/botan/linux/clang/x86_64/botan_all.h \
        cryptography/botan/linux/clang/x86_64/botan_all_internal.h

      debug {
        message(Debug)
        DESTDIR = ../build/linux/clang/x86_64/debug/Kryvos/
      }
      release {
        message(Release)
        DESTDIR = ../build/linux/clang/x86_64/release/Kryvos/
      }
    }

    linux-g++-64 {
      message(Linux g++ x86_64)

      SOURCES += \
        cryptography/botan/linux/gcc/x86_64/botan_all.cpp \
        cryptography/botan/linux/gcc/x86_64/botan_all_aesni.cpp \
        cryptography/botan/linux/gcc/x86_64/botan_all_avx2.cpp \
        cryptography/botan/linux/gcc/x86_64/botan_all_rdrand.cpp \
        cryptography/botan/linux/gcc/x86_64/botan_all_rdseed.cpp \
        cryptography/botan/linux/gcc/x86_64/botan_all_ssse3.cpp
      HEADERS += \
        cryptography/botan/linux/gcc/x86_64/botan_all.h \
        cryptography/botan/linux/gcc/x86_64/botan_all_internal.h

      debug {
        message(Debug)
        DESTDIR = ../build/linux/gcc/x86_64/debug/Kryvos/
      }
      release {
        message(Release)
        DESTDIR = ../build/linux/gcc/x86_64/release/Kryvos/
      }
    }

    linux-g++-32 {
      message(Linux g++ x86)

      SOURCES += \
        cryptography/botan/linux/gcc/x86/botan_all.cpp \
        cryptography/botan/linux/gcc/x86/botan_all_aesni.cpp \
        cryptography/botan/linux/gcc/x86/botan_all_avx2.cpp \
        cryptography/botan/linux/gcc/x86/botan_all_rdrand.cpp \
        cryptography/botan/linux/gcc/x86/botan_all_rdseed.cpp \
        cryptography/botan/linux/gcc/x86/botan_all_ssse3.cpp
      HEADERS += \
        cryptography/botan/linux/gcc/x86/botan_all.h \
        cryptography/botan/linux/gcc/x86/botan_all_internal.h

      debug {
        message(Debug)
        DESTDIR = ../build/linux/gcc/x86/debug/Kryvos/
      }
      release {
        message(Release)
        DESTDIR = ../build/linux/gcc/x86/release/Kryvos/
      }
    } # End linux-g++-32
  } # End Linux

  macx {
    message(macOS clang x86_64)

    CONFIG += c++14

    QMAKE_MAC_SDK = macosx10.12

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector

    SOURCES += \
      cryptography/botan/macOS/clang/x86_64/botan_all.cpp \
      cryptography/botan/macOS/clang/x86_64/botan_all_aesni.cpp \
      cryptography/botan/macOS/clang/x86_64/botan_all_avx2.cpp \
      cryptography/botan/macOS/clang/x86_64/botan_all_rdrand.cpp \
      cryptography/botan/macOS/clang/x86_64/botan_all_rdseed.cpp \
      cryptography/botan/macOS/clang/x86_64/botan_all_ssse3.cpp

    HEADERS += \
      cryptography/botan/macOS/clang/x86_64/botan_all.h \
      cryptography/botan/macOS/clang/x86_64/botan_all_internal.h

    ICON = resources/macOS/icon/Kryvos.icns

    debug {
      message(Debug)
      DESTDIR = ../build/macOS/clang/x86_64/debug/
    }
    release {
      message(Release)
      DESTDIR = ../build/macOS/clang/x86_64/release/
    }
  }

  win32 {
    win32-msvc2015 {
      LIBS += advapi32.lib user32.lib
      QMAKE_CXXFLAGS += -bigobj -arch:AVX2

      contains(QT_ARCH, x86_64) {
        message(Windows MSVC x86_64)

        SOURCES += \
          cryptography/botan/windows/msvc/x86_64/botan_all.cpp \
          cryptography/botan/windows/msvc/x86_64/botan_all_aesni.cpp \
          cryptography/botan/windows/msvc/x86_64/botan_all_avx2.cpp \
          cryptography/botan/windows/msvc/x86_64/botan_all_rdrand.cpp \
          cryptography/botan/windows/msvc/x86_64/botan_all_rdseed.cpp \
          cryptography/botan/windows/msvc/x86_64/botan_all_ssse3.cpp
        HEADERS += \
          cryptography/botan/windows/msvc/x86_64/botan_all.h \
          cryptography/botan/windows/msvc/x86_64/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../build/windows/msvc/x86_64/debug/Kryvos/
        }
        release {
          message(Release)
          DESTDIR = ../build/windows/msvc/x86_64/release/Kryvos/
        }
      } else {
        message(Windows MSVC x86)
        SOURCES += \
          cryptography/botan/windows/msvc/x86/botan_all.cpp \
          cryptography/botan/windows/msvc/x86/botan_all_aesni.cpp \
          cryptography/botan/windows/msvc/x86/botan_all_avx2.cpp \
          cryptography/botan/windows/msvc/x86/botan_all_rdrand.cpp \
          cryptography/botan/windows/msvc/x86/botan_all_rdseed.cpp \
          cryptography/botan/windows/msvc/x86/botan_all_ssse3.cpp

        HEADERS += \
          cryptography/botan/windows/msvc/x86/botan_all.h \
          cryptography/botan/windows/msvc/x86/botan_all_internal.h

        debug {
          message(Debug)
          DESTDIR = ../build/windows/msvc/x86/debug/Kryvos/
        }
        release {
          message(Release)
          DESTDIR = ../build/windows/msvc/x86/release/Kryvos/
        }
      }
    }

    RC_ICONS += resources/windows/icon/kryvos.ico
  } # End win32
} # End desktop

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/qrc

RESOURCES += resources/kryvos.qrc
