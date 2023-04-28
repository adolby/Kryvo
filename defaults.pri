DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD/src/core
INCLUDEPATH += $$PWD/3rdparty/zlib
INCLUDEPATH += $$PWD/3rdparty/doctest
SRC_DIR = $$PWD

linux {
  isEmpty(OPENSSL_INCLUDE_PATH) {
    OPENSSL_INCLUDE_PATH = /usr/include
  }

  isEmpty(OPENSSL_LIB_PATH) {
    OPENSSL_LIB_PATH = /usr/lib/x86_64-linux-gnu
  }
}

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
      OPENSSL_INCLUDE_PATH = %userprofile%/OpenSSLv3/Win_x64/include
    }

    isEmpty(OPENSSL_LIB_PATH) {
      OPENSSL_LIB_PATH = %userprofile%/OpenSSLv3/Win_x64/lib
    }
  }
}
