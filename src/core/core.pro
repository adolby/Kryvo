include(../../defaults.pri)

QT += core

TARGET = core

TEMPLATE = lib

CONFIG += staticlib c++17

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
  Constants.cpp \
  DispatchQueue.cpp \
  SchedulerState.cpp \
  Plugin.cpp \
  PluginLoader.cpp \
  Scheduler.cpp \
  Pipe.cpp \
  archive/Archiver.cpp \
  cryptography/Crypto.cpp \
  settings/Settings.cpp \
  models/FileListModel.cpp \
  models/FileItem.cpp

HEADERS += \
  Constants.hpp \
  DispatchQueue.hpp \
  SchedulerState.hpp \
  FileUtility.h \
  Plugin.hpp \
  PluginLoader.hpp \
  Scheduler.hpp \
  Pipe.hpp \
  archive/Archiver.hpp \
  archive/CompressFileConfig.hpp \
  archive/DecompressFileConfig.hpp \
  cryptography/Crypto.hpp \
  cryptography/CryptoProvider.hpp \
  cryptography/DecryptConfig.hpp \
  cryptography/DecryptFileConfig.hpp \
  cryptography/EncryptConfig.hpp \
  cryptography/EncryptFileConfig.hpp \
  settings/Settings.hpp \
  models/FileListModel.hpp \
  models/FileItem.hpp \
  utility/Thread.hpp \
  utility/pimpl.h

# Platform-specific configuration
linux {
  message(Linux)

  android {
    message(Android)

    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
      message(armeabi-v7a)

      debug {
        message(Debug)
        DESTDIR = $$PWD/../../build/android/armv7/debug/core
      }
      release {
        message(Release)
        DESTDIR = $$PWD/../../build/android/armv7/release/core
      }
    }

    contains(ANDROID_TARGET_ARCH, arm64-v8a) {
      message(arm64-v8a)

      debug {
        message(Debug)
        DESTDIR = $$PWD/../../build/android/arm64_v8a/debug/core
      }
      release {
        message(Release)
        DESTDIR = $$PWD/../../build/android/arm64_v8a/release/core
      }
    }
  } # End android

  linux-clang {
    message(clang)

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/debug/core
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/release/core
    }
  } # End clang

  linux-g++ {
    message(g++)

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/debug/core
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/release/core
    }
  } # End linux-g++
} # End linux

darwin {
  ios {
    message(iOS)
    message(clang)

    CONFIG -= simulator

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../build/iOS/debug/core
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../build/iOS/release/core
    }
  } # End ios

  macos {
    message(macOS)
    message(clang)

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/debug/core
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/release/core
    }
  } # End macos
} # End darwin

win32 {
  message(Windows)

  win32-msvc {
    message(MSVC)

    LIBS += -ladvapi32 -luser32 -lws2_32

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(x86_64)

      debug {
        message(Debug)
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/debug/core
      }
      release {
        message(Release)
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/release/core
      }
    }
  } # End win32-msvc
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc
