QT += core gui widgets

TARGET = kryvos

TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += ../include/common

unix {
    INCLUDEPATH += ../include/linux/x86_64
    LIBS += -L../../../../lib/linux/x86_64

#    INCLUDEPATH += ../include/linux/x86
#    LIBS += -L../../../../lib/linux/x86
}
win32 {
    INCLUDEPATH += ../include/windows/x64
    LIBS += -L../../../../lib/windows/x64

#    INCLUDEPATH += ../include/windows/x86
#    LIBS += -L../../../../lib/windows/x86
}

LIBS += -lbotan-1.11

SOURCES += main.cpp \
           Kryvos.cpp \
           cryptography/Crypto.cpp \
           gui/MainWindow.cpp \
           gui/HeaderFrame.cpp \
           gui/FileListFrame.cpp \
           gui/Delegate.cpp \
           gui/MessageFrame.cpp \
           gui/PasswordFrame.cpp \
           gui/ControlButtonFrame.cpp

HEADERS += Kryvos.hpp \
           cryptography/Crypto.hpp \
           gui/MainWindow.hpp \
           gui/HeaderFrame.hpp \
           gui/FileListFrame.hpp \
           gui/Delegate.hpp \
           gui/MessageFrame.hpp \
           gui/PasswordFrame.hpp \
           gui/ControlButtonFrame.hpp \
           utility/make_unique.h

RESOURCES += resources/kryvos.qrc

QMAKE_CXXFLAGS += -fstack-protector
QMAKE_LFLAGS += -fstack-protector

RC_FILE = resources/rc/kryvos.rc
