QT += core network testlib
TEMPLATE = app
TARGET = test_abstractscraper
DEPENDPATH += .
INCLUDEPATH += ../../src
CONFIG += debug
QT += core network xml
QMAKE_CXXFLAGS += -std=c++17

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
PREFIX = /usr/local
DEFINES+=PREFIX=\\\"$$PREFIX\\\"

include(../../VERSION)
DEFINES+=TESTING
DEFINES+=VERSION=\\\"$$VERSION\\\"

HEADERS += ../../src/abstractscraper.h \
           ../../src/config.h \
           ../../src/gameentry.h \
           ../../src/nametools.h \
           ../../src/netcomm.h \
           ../../src/netmanager.h \
           ../../src/platform.h \
           ../../src/strtools.h 

SOURCES += test_abstractscraper.cpp \
           ../../src/abstractscraper.cpp \
           ../../src/config.cpp \
           ../../src/gameentry.cpp \
           ../../src/nametools.cpp \
           ../../src/netcomm.cpp \
           ../../src/netmanager.cpp \
           ../../src/platform.cpp \
           ../../src/strtools.cpp 

