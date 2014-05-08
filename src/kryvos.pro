QT += core gui widgets svg

TARGET = kryvos

TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += ../include/windows/x64

LIBS += -L../../../lib/windows/x64 -lbotan-1.11

#INCLUDEPATH += ../include/windows/x86
#LIBS += -L../../../lib/windows/x86 -lbotan-1.11

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

RC_FILE = resources/rc/kryvos.rc
