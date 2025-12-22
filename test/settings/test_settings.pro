QT += core network xml testlib
TEMPLATE = app
TARGET = test_settings
DEPENDPATH += .
INCLUDEPATH += ../../src
CONFIG += debug
QMAKE_CXXFLAGS += -std=c++17

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
PREFIX = /usr/local
DEFINES+=PREFIX=\\\"$$PREFIX\\\"

include(../../VERSION.ini)
DEFINES+=VERSION=\\\"$$VERSION\\\"

QMAKE_POST_LINK += cp -f $$shell_quote($$shell_path($${PWD}/../../peas.json)) .; 
QMAKE_POST_LINK += cp -f $$shell_quote($$shell_path($${PWD}/../../platforms_idmap.csv)) .;

# Input
HEADERS += ../../src/cache.h \
           ../../src/cli.h \
           ../../src/config.h \
           ../../src/gameentry.h \
           ../../src/nametools.h \
           ../../src/platform.h \
           ../../src/queue.h \
           ../../src/settings.h \
           ../../src/strtools.h
SOURCES += test_settings.cpp \
           ../../src/cache.cpp \
           ../../src/cli.cpp \
           ../../src/config.cpp \
           ../../src/gameentry.cpp \
           ../../src/nametools.cpp \
           ../../src/platform.cpp \
           ../../src/queue.cpp \           
           ../../src/settings.cpp \
           ../../src/strtools.cpp