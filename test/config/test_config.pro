TEMPLATE = app
TARGET = test_config
DEPENDPATH += .
INCLUDEPATH += ../../src
CONFIG += debug
QT += core network xml testlib
QMAKE_CXXFLAGS += -std=c++17

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

PREFIX = $$(PWD)/usr/local
SYSCONFDIR = $${PREFIX}/etc
DEFINES+=PREFIX=\\\"$$PREFIX\\\"
DEFINES+=SYSCONFDIR=\\\"$$SYSCONFDIR\\\"
DEFINES+=TESTING

include(../../VERSION.ini)
DEFINES+=VERSION=\\\"$$VERSION\\\"

HEADERS += \
            ../../src/config.h \
            ../../src/nocolor.h \
            ../../src/pathtools.h \
            ../../src/platform.h

SOURCES += test_config.cpp \
            ../../src/config.cpp \
            ../../src/nocolor.cpp \
            ../../src/pathtools.cpp \
            ../../src/platform.cpp

