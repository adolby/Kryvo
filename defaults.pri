DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD/src/core
INCLUDEPATH += $$PWD/3rdparty/zlib
INCLUDEPATH += $$PWD/3rdparty/doctest
SRC_DIR = $$PWD

linux {
  isEmpty(OPENSSL_PATH) {
    OPENSSL_PATH = /usr/local/ssl
  }
}

darwin {
  isEmpty(OPENSSL_PATH) {
    OPENSSL_PATH = /usr/local/opt/openssl@3
  }
}

win32 {
  contains(QT_ARCH, x86_64) {
    isEmpty(OPENSSL_PATH) {
      OPENSSL_PATH = %userprofile%/OpenSSLv3/Win_x64
    }
  }
}
