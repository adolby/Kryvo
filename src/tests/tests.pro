QT += core testlib

TARGET = tests

TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += .. \
               ../../include/x64

LIBS += -L../../../../lib/x64 -lbotan-1.11

# Input
SOURCES += ../cryptography/Crypto.cpp \
           test_Crypto.cpp

HEADERS += ../cryptography/Crypto.hpp \
           test_Crypto.hpp
