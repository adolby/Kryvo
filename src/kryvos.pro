QT += core gui widgets

TARGET = kryvos

TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += ../include/common

unix {
    INCLUDEPATH += ../include/linux/x86_64
    LIBS += -L../../../../lib/linux/x86_64

#    INCLUDEPATH += ../include/linux/x86
#    LIBS += -L../../../lib/linux/x86
}
win32 {
    INCLUDEPATH += ../include/windows/x64
    LIBS += -L../../../lib/windows/x64

#    INCLUDEPATH += ../include/windows/x86
#    LIBS += -L../../../lib/windows/x86
}

LIBS += -lbotan-1.11

SOURCES += \
    main.cpp \
    Kryvos.cpp \
    gui/MainWindow.cpp \
    gui/Delegate.cpp \
    cryptography/Crypto.cpp

HEADERS += \
    Kryvos.hpp \
    gui/MainWindow.hpp \
    gui/Delegate.hpp \
    cryptography/Crypto.hpp

RESOURCES += resources/kryvos.qrc

QMAKE_CXXFLAGS += -fstack-protector
QMAKE_LFLAGS += -fstack-protector

RC_FILE = resources/rc/kryvos.rc
