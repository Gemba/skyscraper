TEMPLATE = app
TARGET = test_config
DEPENDPATH += .
INCLUDEPATH += ../../src
CONFIG += debug
QT += core network xml testlib
QMAKE_CXXFLAGS += -std=c++17

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

PREFIX = /usr/local
DEFINES+=PREFIX=\\\"$$PREFIX\\\"

include(../../VERSION.ini)
DEFINES+=VERSION=\\\"$$VERSION\\\"

HEADERS += ../../src/config.h \
            ../../src/platform.h \
            ../../src/strtools.h

SOURCES += test_config.cpp \
           ../../src/config.cpp \
           ../../src/platform.cpp \
           ../../src/strtools.cpp


