TEMPLATE = app
TARGET = test_strtools
DEPENDPATH += .
INCLUDEPATH += ../../src
CONFIG += debug
QT += core network xml testlib
QMAKE_CXXFLAGS += -std=c++17

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

include(../../VERSION)
DEFINES+=VERSION=\\\"$$VERSION\\\"

HEADERS += ../../src/strtools.h 

SOURCES += test_strtools.cpp \
           ../../src/strtools.cpp 

