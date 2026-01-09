QT += core network testlib
TEMPLATE = app
TARGET = test_pegasus
DEPENDPATH += .
INCLUDEPATH += ../../src
CONFIG += debug
QT += core network xml
QMAKE_CXXFLAGS += -std=c++17

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
PREFIX = /usr/local
DEFINES+=PREFIX=\\\"$$PREFIX\\\"

include(../../VERSION.ini)
DEFINES+=TESTING
DEFINES+=VERSION=\\\"$$VERSION\\\"

HEADERS += ../../src/pegasus.h \
           ../../src/abstractfrontend.h \
           ../../src/config.h \
           ../../src/gameentry.h \
           ../../src/platform.h

SOURCES += test_pegasus.cpp \
           ../../src/pegasus.cpp \
           ../../src/abstractfrontend.cpp \
           ../../src/config.cpp \
           ../../src/gameentry.cpp \
           ../../src/platform.cpp

