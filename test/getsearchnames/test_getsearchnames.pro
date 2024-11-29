QT += core network xml testlib
TEMPLATE = app
TARGET = test_getsearchnames
DEPENDPATH += .
INCLUDEPATH += ../../src
CONFIG += debug
QMAKE_CXXFLAGS += -std=c++17

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
PREFIX = /usr/local
DEFINES+=PREFIX=\\\"$$PREFIX\\\"

include(../../VERSION)
DEFINES+=VERSION=\\\"$$VERSION\\\"

# set if testing against v3.10.3 or earlier
###DEFINES+=VER_3_10_3

HEADERS += ../../src/abstractscraper.h  \ 
             ../../src/arcadedb.h \
             ../../src/cache.h \
             ../../src/cli.h \
             ../../src/config.h \
             ../../src/crc32.h \
             ../../src/esgamelist.h \
             ../../src/gameentry.h \
             ../../src/igdb.h \
             ../../src/mobygames.h \
             ../../src/nametools.h \
             ../../src/netcomm.h \
             ../../src/netmanager.h \
             ../../src/openretro.h \
             ../../src/platform.h \
             ../../src/queue.h \ 
             ../../src/screenscraper.h \
             ../../src/settings.h \
             ../../src/strtools.h 

SOURCES +=  test_getsearchnames.cpp \
             ../../src/abstractscraper.cpp \
             ../../src/arcadedb.cpp \
             ../../src/cache.cpp \
             ../../src/cli.cpp \
             ../../src/config.cpp \
             ../../src/crc32.cpp \
             ../../src/esgamelist.cpp \
             ../../src/gameentry.cpp \
             ../../src/igdb.cpp \
             ../../src/mobygames.cpp \
             ../../src/nametools.cpp \
             ../../src/netcomm.cpp \
             ../../src/netmanager.cpp \
             ../../src/openretro.cpp \
             ../../src/platform.cpp \
             ../../src/queue.cpp \
             ../../src/screenscraper.cpp \
             ../../src/settings.cpp \
             ../../src/strtools.cpp
