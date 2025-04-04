TEMPLATE = app
TARGET = test_nametools
DEPENDPATH += .
INCLUDEPATH += ../../src
CONFIG += debug
QT += core network xml testlib
QMAKE_CXXFLAGS += -std=c++17

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

include(../../VERSION.ini)
DEFINES+=VERSION=\\\"$$VERSION\\\"

HEADERS += ../../src/nametools.h \
            ../../src/strtools.h 

SOURCES += test_nametools.cpp \
           ../../src/nametools.cpp \
           ../../src/strtools.cpp 


