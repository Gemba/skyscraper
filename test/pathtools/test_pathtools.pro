TEMPLATE = app
TARGET = test_pathtools
DEPENDPATH += .
INCLUDEPATH += ../../src
CONFIG += debug
QT += core network xml testlib
QMAKE_CXXFLAGS += -std=c++17

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

unix {
  # for GCC8 (RetroPie Buster)
  system( g++ --version | grep "^g++" | grep -c "8.3." >/dev/null ) {
    LIBS += -lstdc++fs
  }
}

PREFIX = /usr/local
SYSCONFDIR = $${PREFIX}/etc
DEFINES+=PREFIX=\\\"$$PREFIX\\\"
DEFINES+=SYSCONFDIR=\\\"$$SYSCONFDIR\\\"

include(../../VERSION.ini)
DEFINES+=VERSION=\\\"$$VERSION\\\"

HEADERS += \
           ../../src/config.h \
           ../../src/pathtools.h \
           ../../src/platform.h \
           ../../src/strtools.h

SOURCES += test_pathtools.cpp \
           ../../src/config.cpp \
           ../../src/pathtools.cpp \
           ../../src/platform.cpp \
           ../../src/strtools.cpp


