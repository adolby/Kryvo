include(../../defaults.pri)

QT += core gui qml quick quickcontrols2

TARGET = Kryvo

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
  Application.cpp \
  Ui.cpp \
  FileListModel.cpp \
  FileItem.cpp

HEADERS += \
  Application.hpp \
  Ui.hpp \
  FileListModel.hpp \
  FileItem.hpp

# Platform-specific configuration
linux {
  message(Linux)

  android {
    message(Android)

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../../resources/android

    DISTFILES += \
      ../../resources/android/AndroidManifest.xml \
      ../../resources/android/build.gradle \
      ../../resources/android/gradle/wrapper/gradle-wrapper.jar \
      ../../resources/android/gradle/wrapper/gradle-wrapper.properties \
      ../../resources/android/gradlew \
      ../../resources/android/gradlew.bat \
      ../../resources/android/res/values/libs.xml

    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
      message(armeabi-v7a)

      debug {
        message(Debug)
        LIBS += -L$$PWD/../../build/android/armv7/debug/core -lcore
        LIBS += -L$$PWD/../../build/android/armv7/debug/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/armv7/debug/plugins/cryptography/botan -lbotan
        DESTDIR = $$PWD/../../build/android/armv7/debug/quick
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/android/armv7/release/core -lcore
        LIBS += -L$$PWD/../../build/android/armv7/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/armv7/release/plugins/cryptography/botan -lbotan
        DESTDIR = $$PWD/../../build/android/armv7/release/quick
      }
    }

    contains(ANDROID_TARGET_ARCH, arm64-v8a) {
      message(arm64-v8a)

      debug {
        message(Debug)
        LIBS += -L$$PWD/../../build/android/armv8/debug/core -lcore
        LIBS += -L$$PWD/../../build/android/armv8/debug/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/armv8/debug/plugins/cryptography/botan -lbotan
        DESTDIR = $$PWD/../../build/android/armv8/debug/quick
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/android/armv8/release/core -lcore
        LIBS += -L$$PWD/../../build/android/armv8/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/android/armv8/release/plugins/cryptography/botan -lbotan
        DESTDIR = $$PWD/../../build/android/armv8/release/quick
      }
    }
  } # End android

  linux-clang {
    message(clang)

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/clang/x86_64/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/release/quick
    }
  } # End linux-clang

  linux-g++-64 {
    message(g++ x86_64)

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector
    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/linux/gcc/x86_64/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/release/quick
    }
  } # End linux-g++-64
} # End linux

darwin {
  ios {
    message(iOS)
    message(clang)

    CONFIG -= simulator

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/iOS/debug/core -lcore
      LIBS += -L$$PWD/../../build/iOS/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/iOS/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/iOS/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/iOS/release/core -lcore
      LIBS += -L$$PWD/../../build/iOS/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/iOS/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/iOS/release/quick
    }

    LIBS += -framework Foundation -framework CoreFoundation -framework UIKit
  } # End ios

  macos {
    message(macOS)
    message(clang)

    QMAKE_CXXFLAGS += -fstack-protector -maes -mpclmul -mssse3 -mavx2
    QMAKE_LFLAGS += -fstack-protector

    QMAKE_TARGET_BUNDLE_PREFIX = app.kryvo
    ICON = $$PWD/../../resources/icons/kryvo.icns

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/core -lcore
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/core -lcore
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/macOS/clang/x86_64/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/release/quick
    }
  } # End macos
} # End darwin

win32 {
  message(Windows)

  win32-g++ {
    message(g++)

    debug {
      message(Debug)
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/debug/core -lcore
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/debug/lib/zlib -lz
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/debug/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/windows/mingw/x86_32/debug/quick
    }
    release {
      message(Release)
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/release/core -lcore
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/release/lib/zlib -lz
      LIBS += -L$$PWD/../../build/windows/mingw/x86_32/release/plugins/cryptography/botan -lbotan
      DESTDIR = $$PWD/../../build/windows/mingw/x86_32/release/quick
    }
  } # End win32-g++

  win32-msvc {
    message(MSVC)

    LIBS += -ladvapi32 -luser32 -lws2_32

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(x86_64)

      debug {
        message(Debug)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/core -lcore
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/lib/zlib -lz
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/debug/plugins/cryptography/botan -lbotan
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/debug/quick
      }
      release {
        message(Release)
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/core -lcore
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/lib/zlib -lz
        LIBS += -L$$PWD/../../build/windows/msvc/x86_64/release/plugins/cryptography/botan -lbotan
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/release/quick
      }
    }
  } # End win32-msvc

  RC_ICONS += $$PWD/../../resources/icons/kryvo.ico
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc

RESOURCES += $$PWD/../../resources/images.qrc $$PWD/quick.qrc
