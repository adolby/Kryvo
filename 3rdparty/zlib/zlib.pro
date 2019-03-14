include(../../defaults.pri)

TARGET = z

TEMPLATE = lib

CONFIG += staticlib c++14

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
  adler32.c \
  compress.c \
  crc32.c \
  deflate.c \
  gzclose.c \
  gzlib.c \
  gzread.c \
  gzwrite.c \
  infback.c \
  inffast.c \
  inflate.c \
  inftrees.c \
  trees.c \
  uncompr.c \
  zutil.c

HEADERS += \
  crc32.h \
  deflate.h \
  gzguts.h \
  inffast.h \
  inffixed.h \
  inflate.h \
  inftrees.h \
  trees.h \
  zconf.h \
  zlib.h \
  zutil.h

# Platform-specific configuration
linux {
  message(Linux)

  QMAKE_CXXFLAGS += -fstack-protector
  QMAKE_LFLAGS += -fstack-protector

  android {
    message(Android)

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../build/android/debug/lib/zlib/
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../build/android/release/lib/zlib/
    }
  } # End Android

  linux-clang {
    message(clang)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/debug/lib/zlib/
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../build/linux/clang/x86_64/release/lib/zlib/
    }
  } # End clang

  linux-g++ {
    message(g++)

    QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/debug/lib/zlib/
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../build/linux/gcc/x86_64/release/lib/zlib/
    }
  } # End g++
} # End Linux

darwin {
  QMAKE_CXXFLAGS += -fstack-protector
  QMAKE_LFLAGS += -fstack-protector

  ios {
    message(iOS)
    message(clang)

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../build/iOS/debug/lib/zlib/
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../build/iOS/release/lib/zlib/
    }
  } # End ios

  macos {
    message(macOS)
    message(clang)

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/debug/lib/zlib/
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../build/macOS/clang/x86_64/release/lib/zlib/
    }
  } # End macos
} # End darwin

win32 {
  message(Windows)

  win32-g++ {
    message(g++)

    debug {
      message(Debug)
      DESTDIR = $$PWD/../../build/windows/mingw/x86_32/debug/lib/zlib/
    }
    release {
      message(Release)
      DESTDIR = $$PWD/../../build/windows/mingw/x86_32/release/lib/zlib/
    }
  }

  win32-msvc {
    message(MSVC)

    QMAKE_CXXFLAGS += -bigobj -arch:AVX2

    contains(QT_ARCH, x86_64) {
      message(x86_64)

      debug {
        message(Debug)
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/debug/lib/zlib/
      }
      release {
        message(Release)
        DESTDIR = $$PWD/../../build/windows/msvc/x86_64/release/lib/zlib/
      }
    }
  }
} # End win32

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
