QT += core network xml testlib
TEMPLATE = app
TARGET = test_retroarch
DEPENDPATH += .
INCLUDEPATH += ../../src
CONFIG += debug
QMAKE_CXXFLAGS += -std=c++17

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
PREFIX = /usr/local
SYSCONFDIR = $${PREFIX}/etc
DEFINES+=PREFIX=\\\"$$PREFIX\\\"
DEFINES+=SYSCONFDIR=\\\"$$SYSCONFDIR\\\"

include(../../VERSION.ini)
DEFINES+=TESTING
DEFINES+=VERSION=\\\"$$VERSION\\\"

HEADERS += \
            ../../src/retroarch.h \
            ../../src/abstractfrontend.h \
            ../../src/config.h \
            ../../src/gameentry.h \
            ../../src/nocolor.h \
            ../../src/pathtools.h \
            ../../src/platform.h \
            ../../src/strtools.h

SOURCES += test_retroarch.cpp \
           ../../src/retroarch.cpp \
           ../../src/abstractfrontend.cpp \
           ../../src/config.cpp \
           ../../src/gameentry.cpp \
           ../../src/nocolor.cpp \
           ../../src/pathtools.cpp \
           ../../src/platform.cpp \
           ../../src/strtools.cpp \

# Test data directory
DISTFILES += playlists/test.lpl