TEMPLATE = app
TARGET = Skyscraper
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += release
# enable for XDG directory layout - see also Skyscraper docs
#DEFINES+=XDG
# set std-C++17 for clang and gcc
CONFIG += c++1z
QT += core network sql xml

# Installation prefix path for bin/Skyscraper and etc/skyscraper/*
PREFIX=$$(PREFIX)
# One time set with "PREFIX=/path/to qmake"?
isEmpty(PREFIX) {
  # No. Try qmake persistent property $$[PREFIX].
  PREFIX = $$[PREFIX]
}
# Check if persistent property has been set with "qmake -set PREFIX /path/to"?
isEmpty(PREFIX) {
  # No. Use default.
  PREFIX = /usr/local
}

unix:target.path=$${PREFIX}/bin
unix:target.files=Skyscraper Skyscraper.app/Contents/MacOS/Skyscraper

unix:supplementary.path=$${PREFIX}/bin
unix:supplementary.files=\
  supplementary/scraperdata/check_screenscraper_json_to_idmap.py \
  supplementary/scraperdata/convert_platforms_json.py \
  supplementary/scraperdata/peas_and_idmap_verify.py

unix:config.path=$${PREFIX}/etc/skyscraper
unix:config.files=aliasMap.csv hints.xml mameMap.csv \
  mobygames_platforms.json peas.json platforms_idmap.csv \
  screenscraper_platforms.json tgdb_developers.json \
  tgdb_genres.json tgdb_platforms.json tgdb_publishers.json

unix:examples.path=$${PREFIX}/etc/skyscraper
unix:examples.files=config.ini.example README.md artwork.xml \
  artwork.xml.example1 artwork.xml.example2 artwork.xml.example3 \
  artwork.xml.example4 docs/ARTWORK.md docs/CACHE.md

unix:cacheexamples.path=$${PREFIX}/etc/skyscraper/cache
unix:cacheexamples.files=cache/priorities.xml.example docs/CACHE.md

unix:impexamples.path=$${PREFIX}/etc/skyscraper/import
unix:impexamples.files=docs/IMPORT.md import/definitions.dat.example1 \
  import/definitions.dat.example2

unix:resexamples.path=$${PREFIX}/etc/skyscraper/resources
unix:resexamples.files=resources/maskexample.png resources/frameexample.png \
  resources/boxfront.png resources/boxside.png resources/scanlines1.png \
  resources/scanlines2.png

unix:INSTALLS += target config examples cacheexamples impexamples \
  resexamples supplementary

include(./VERSION.ini)
unix:dev=$$find(VERSION, "-dev")
unix:count(dev, 1) {
  rev=$$system(git describe --always)
  VERSION=$$replace(VERSION, "dev", $$rev)
}
DEFINES+=VERSION=\\\"$$VERSION\\\"
DEFINES+=PREFIX=\\\"$$PREFIX\\\"

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

include(win32/skyscraper.pro)

HEADERS += src/skyscraper.h \
           src/netmanager.h \
           src/netcomm.h \
           src/xmlreader.h \
           src/settings.h \
           src/cli.h \
           src/compositor.h \
           src/config.h \
           src/strtools.h \
           src/imgtools.h \
           src/esgamelist.h \
           src/scraperworker.h \
           src/cache.h \
           src/localscraper.h \
           src/importscraper.h \
           src/gameentry.h \
           src/abstractscraper.h \
           src/abstractfrontend.h \
           src/emulationstation.h \
           src/esde.h \
           src/attractmode.h \
           src/pegasus.h \
           src/openretro.h \
           src/thegamesdb.h \
           src/zxinfodk.h \
           src/screenscraper.h \
           src/crc32.h \
           src/mobygames.h \
           src/gamebase.h \
           src/igdb.h \
           src/arcadedb.h \
           src/platform.h \
           src/layer.h \
           src/fxshadow.h \
           src/fxblur.h \
           src/fxmask.h \
           src/fxframe.h \
           src/fxrounded.h \
           src/fxstroke.h \
           src/fxbrightness.h \
           src/fxcontrast.h \
           src/fxbalance.h \
           src/fxopacity.h \
           src/fxgamebox.h \
           src/fxhue.h \
           src/fxsaturation.h \
           src/fxcolorize.h \
           src/fxrotate.h \
           src/fxscanlines.h \
           src/nametools.h \
           src/queue.h

SOURCES += src/main.cpp \
           src/skyscraper.cpp \
           src/netmanager.cpp \
           src/netcomm.cpp \
           src/xmlreader.cpp \
           src/settings.cpp \
           src/cli.cpp \
           src/compositor.cpp \
           src/config.cpp \
           src/strtools.cpp \
           src/imgtools.cpp \
           src/esgamelist.cpp \
           src/scraperworker.cpp \
           src/cache.cpp \
           src/localscraper.cpp \
           src/importscraper.cpp \
           src/gameentry.cpp \
           src/abstractscraper.cpp \
           src/abstractfrontend.cpp \
           src/emulationstation.cpp \
           src/esde.cpp \
           src/attractmode.cpp \
           src/pegasus.cpp \
           src/openretro.cpp \
           src/thegamesdb.cpp \
           src/zxinfodk.cpp \
           src/screenscraper.cpp \
           src/crc32.cpp \
           src/mobygames.cpp \
           src/gamebase.cpp \
           src/igdb.cpp \
           src/arcadedb.cpp \
           src/platform.cpp \
           src/layer.cpp \
           src/fxshadow.cpp \
           src/fxblur.cpp \
           src/fxmask.cpp \
           src/fxframe.cpp \
           src/fxrounded.cpp \
           src/fxstroke.cpp \
           src/fxbrightness.cpp \
           src/fxcontrast.cpp \
           src/fxbalance.cpp \
           src/fxopacity.cpp \
           src/fxgamebox.cpp \
           src/fxhue.cpp \
           src/fxsaturation.cpp \
           src/fxcolorize.cpp \
           src/fxrotate.cpp \
           src/fxscanlines.cpp \
           src/nametools.cpp \
           src/queue.cpp

SUBDIRS += \
    win32/skyscraper.pro
