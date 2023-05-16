DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD/src/core
INCLUDEPATH += $$PWD/3rdparty/zlib
INCLUDEPATH += $$PWD/3rdparty/doctest
SRC_DIR = $$PWD

linux {
  android {
    isEmpty(OPENSSL_INCLUDE_PATH) {
      OPENSSL_INCLUDE_PATH = $(HOME)/android_openssl/ssl_3/include/
    }

    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
      isEmpty(OPENSSL_LIB_PATH) {
        OPENSSL_LIB_PATH = $(HOME)/android_openssl/ssl_3/armeabi-v7a/
      }
    }

    contains(ANDROID_TARGET_ARCH, arm64-v8a) {
      isEmpty(OPENSSL_LIB_PATH) {
        OPENSSL_LIB_PATH = $(HOME)/android_openssl/ssl_3/arm64-v8a/
      }
    }
  }

  linux-clang {
    isEmpty(OPENSSL_INCLUDE_PATH) {
      OPENSSL_INCLUDE_PATH = /usr/include
    }

    isEmpty(OPENSSL_LIB_PATH) {
      OPENSSL_LIB_PATH = /usr/lib/x86_64-linux-gnu
    }
  }

  linux-g++ {
    isEmpty(OPENSSL_INCLUDE_PATH) {
      OPENSSL_INCLUDE_PATH = /usr/include
    }

    isEmpty(OPENSSL_LIB_PATH) {
      OPENSSL_LIB_PATH = /usr/lib/x86_64-linux-gnu
    }
  }
} # End linux

darwin {
  isEmpty(OPENSSL_INCLUDE_PATH) {
    OPENSSL_INCLUDE_PATH = /usr/local/opt/openssl@3/include
  }

  isEmpty(OPENSSL_LIB_PATH) {
    OPENSSL_LIB_PATH = /usr/local/opt/openssl@3/lib
  }
}

win32 {
  contains(QT_ARCH, x86_64) {
    isEmpty(OPENSSL_INCLUDE_PATH) {
      OPENSSL_INCLUDE_PATH = $$(userprofile)\OpenSSLv3\Win_x64\include
    }

    isEmpty(OPENSSL_LIB_PATH) {
      OPENSSL_LIB_PATH = $$(userprofile)\OpenSSLv3\Win_x64\lib
    }
  }
}
