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

unix {
  # for GCC8 (RetroPie Buster)
  system( g++ --version | grep "^g++" | grep -c "8.3." >/dev/null ) {
    LIBS += -lstdc++fs
  }
}

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

# System configuration directory
SYSCONFDIR = $$(SYSCONFDIR)
isEmpty(SYSCONFDIR) {
  SYSCONFDIR = $${PREFIX}/etc
}

unix:target.path=$${PREFIX}/bin
unix:target.files=Skyscraper Skyscraper.app/Contents/MacOS/Skyscraper

unix:supplementary.path=$${PREFIX}/bin
unix:supplementary.files=\
  supplementary/scraperdata/deepdiff_peas_jsonfiles.py \
  supplementary/scraperdata/mdb2sqlite.sh \
  supplementary/scraperdata/peas_and_idmap_verify.py \
  supplementary/scraperdata/README-Skyscraper-Scripts.md

unix:config.path=$${SYSCONFDIR}/skyscraper
unix:config.files=aliasMap.csv hints.xml mameMap.csv \
  mobygames_platforms.json peas.json platforms_idmap.csv \
  screenscraper_platforms.json tgdb_developers.json \
  tgdb_genres.json tgdb_platforms.json tgdb_publishers.json

unix:examples.path=$${SYSCONFDIR}/skyscraper
unix:examples.files=config.ini.example README.md artwork.xml \
  artwork.xml.example1 artwork.xml.example2 artwork.xml.example3 \
  artwork.xml.example4 batocera-artwork.xml docs/ARTWORK.md docs/CACHE.md

unix:cacheexamples.path=$${SYSCONFDIR}/skyscraper/cache
unix:cacheexamples.files=cache/priorities.xml.example docs/CACHE.md

unix:impexamples.path=$${SYSCONFDIR}/skyscraper/import
unix:impexamples.files=docs/IMPORT.md import/definitions.dat.example1 \
  import/definitions.dat.example2

unix:resexamples.path=$${SYSCONFDIR}/skyscraper/resources
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
DEFINES+=SYSCONFDIR=\\\"$$SYSCONFDIR\\\"

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

include(win32/skyscraper.pro)

HEADERS += \
           src/abstractfrontend.h \
           src/abstractscraper.h \
           src/arcadedb.h \
           src/attractmode.h \
           src/batocera.h \
           src/cache.h \
           src/cli.h \
           src/compositor.h \
           src/config.h \
           src/crc32.h \
           src/emulationstation.h \
           src/esde.h \
           src/esgamelist.h \
           src/fxbalance.h \
           src/fxblur.h \
           src/fxbrightness.h \
           src/fxcolorize.h \
           src/fxcontrast.h \
           src/fxframe.h \
           src/fxgamebox.h \
           src/fxhue.h \
           src/fxmask.h \
           src/fxopacity.h \
           src/fxrotate.h \
           src/fxrounded.h \
           src/fxsaturation.h \
           src/fxscanlines.h \
           src/fxshadow.h \
           src/fxstroke.h \
           src/gamebase.h \
           src/gameentry.h \
           src/igdb.h \
           src/imgtools.h \
           src/importscraper.h \
           src/layer.h \
           src/localscraper.h \
           src/mobygames.h \
           src/nametools.h \
           src/netcomm.h \
           src/netmanager.h \
           src/nocolor.h \
           src/openretro.h \
           src/pathtools.h \
           src/pegasus.h \
           src/platform.h \
           src/queue.h \
           src/retroarch.h \
           src/scraperworker.h \
           src/screenscraper.h \
           src/settings.h \
           src/skyscraper.h \
           src/strtools.h \
           src/thegamesdb.h \
           src/xmlreader.h \
           src/zxinfodk.h

SOURCES += src/main.cpp \
           src/abstractfrontend.cpp \
           src/abstractscraper.cpp \
           src/arcadedb.cpp \
           src/attractmode.cpp \
           src/batocera.cpp \
           src/cache.cpp \
           src/cli.cpp \
           src/compositor.cpp \
           src/config.cpp \
           src/crc32.cpp \
           src/emulationstation.cpp \
           src/esde.cpp \
           src/esgamelist.cpp \
           src/fxbalance.cpp \
           src/fxblur.cpp \
           src/fxbrightness.cpp \
           src/fxcolorize.cpp \
           src/fxcontrast.cpp \
           src/fxframe.cpp \
           src/fxgamebox.cpp \
           src/fxhue.cpp \
           src/fxmask.cpp \
           src/fxopacity.cpp \
           src/fxrotate.cpp \
           src/fxrounded.cpp \
           src/fxsaturation.cpp \
           src/fxscanlines.cpp \
           src/fxshadow.cpp \
           src/fxstroke.cpp \
           src/gamebase.cpp \
           src/gameentry.cpp \
           src/igdb.cpp \
           src/imgtools.cpp \
           src/importscraper.cpp \
           src/layer.cpp \
           src/localscraper.cpp \
           src/mobygames.cpp \
           src/nametools.cpp \
           src/netcomm.cpp \
           src/netmanager.cpp \
           src/nocolor.cpp \
           src/openretro.cpp \
           src/pathtools.cpp \
           src/pegasus.cpp \
           src/platform.cpp \
           src/queue.cpp \
           src/retroarch.cpp \
           src/scraperworker.cpp \
           src/screenscraper.cpp \
           src/settings.cpp \
           src/skyscraper.cpp \
           src/strtools.cpp \
           src/thegamesdb.cpp \
           src/xmlreader.cpp \
           src/zxinfodk.cpp

SUBDIRS += \
    win32/skyscraper.pro
