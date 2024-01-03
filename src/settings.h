/***************************************************************************
 *            settings.h
 *
 *  Wed Jun 14 12:00:00 CEST 2017
 *  Copyright 2017 Lars Muldjord
 *  muldjordlars@gmail.com
 ****************************************************************************/
/*
 *  This file is part of skyscraper.
 *
 *  skyscraper is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  skyscraper is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with skyscraper; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QCommandLineParser>
#include <QDir>
#include <QImage>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QSettings>

struct Settings {
    QString currentDir = "";

    QString cacheFolder = "";
    QString gameListFileString = "";
    QString skippedFileString = "";
    QString configFile = "";
    QString frontend = "emulationstation";
    // For use with 'emulator' with Attractmode and 'command' for Pegasus
    QString frontendExtra = "";
    QString platform = "";
    QString scraper = "";
    QString userCreds = "";
    QString igdbToken = "";
    QString inputFolder = "";
    QString gameListFolder = "";
    QString mediaFolder = "";
    // Only relevant for EmulationStation
    bool mediaFolderHidden = false;
    QString screenshotsFolder = "";
    QString coversFolder = "";
    QString wheelsFolder = "";
    QString marqueesFolder = "";
    QString texturesFolder = "";
    QString videosFolder = "";
    QString importFolder = "import";
    QString nameTemplate = "";
    int doneThreads = 0;
    int threads = 4;
    bool threadsSet = false;
    int minMatch = 65;
    bool minMatchSet = false;
    int notFound = 0;
    int found = 0;
    int currentFile = 0;
    int totalFiles = 0;
    int maxLength = 2500;
    bool brackets = true;
    bool refresh = false;
    QString cacheOptions = "";
    bool cacheResize = true;
    int jpgQuality = 95;
    bool subdirs = true;
    bool onlyMissing = false;
    QString startAt = "";
    QString endAt = "";
    bool pretend = false;
    bool unattend = false;
    bool unattendSkip = false;
    bool interactive = false;
    bool forceFilename = false;
    bool stats = false;
    bool hints = true;
    int verbosity = 0;
    int maxFails = 42;
    bool skipped = false;
    bool tidyDesc = true;
    QString artworkConfig = "artwork.xml";
    QByteArray artworkXml = "";
    QString excludePattern = "";
    QString includePattern = "";
    QString includeFrom = "";
    QString excludeFrom = "";
    bool relativePaths = false;
    QString extensions = "";
    QString addExtensions = "";
    bool unpack = false;
    bool theInFront = false;
    bool gameListBackup = false;
    bool preserveOldGameList = true;
    bool spaceCheck = true;
    QString scummIni = "";

    int romLimit = -1;

    bool videos = false;
    bool videoPreferNormalized = true;
    int videoSizeLimit = 100 * 1000 * 1000;
    QString videoConvertCommand = "";
    QString videoConvertExtension = "";
    bool symlink = false;
    bool skipExistingVideos = false;
    bool cacheCovers = true;
    bool skipExistingCovers = false;
    bool cacheScreenshots = true;
    bool skipExistingScreenshots = false;
    // If true, will crop away black borders around screenshots
    bool cropBlack = true;
    bool cacheWheels = true;
    bool skipExistingWheels = false;
    bool cacheMarquees = true;
    bool skipExistingMarquees = false;
    bool skipExistingTextures = false;
    bool cacheTextures = true;

    QString user = "";
    QString password = "";
    QString lang = "";
    QString region = "";
    QString langPriosStr = "";
    QString regionPriosStr = "";

    QString searchName = "";

    QMap<QString, QImage> resources;
    QMap<QString, QString> mameMap;
    QMap<QString, QString> aliasMap;
    QMap<QString, QPair<QString, QString>> whdLoadMap;

    QList<QString> regionPrios;
    QList<QString> langPrios;
};

class RuntimeCfg : public QObject {
    Q_OBJECT

public:
    enum CfgType { MAIN = 1, PLATFORM = 2, FRONTEND = 4, SCRAPER = 8 };

    RuntimeCfg(Settings *config, const QCommandLineParser *parser);
    ~RuntimeCfg();

    void applyConfigIni(CfgType type, QSettings *settings, bool &inputFolderSet,
                        bool &gameListFolderSet, bool &mediaFolderSet);

    void applyCli(bool &inputFolderSet, bool &gameListFolderSet,
                  bool &mediaFolderSet);

    QString concatPath(QString absPath, QString platformFolder);

private:
    void setFlag(const QString flag);
    QSet<QString> getKeys(CfgType type);
    QStringList parseFlags();

    Settings *config;
    const QCommandLineParser *parser;

    QMap<QString, QPair<QString, int>> params = {
        // clang-format off
        {"addExtensions",         QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"artworkXml",            QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"brackets",              QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"cacheCovers",           QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )},
        {"cacheFolder",           QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"cacheMarquees",         QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )},
        {"cacheRefresh",          QPair<QString, int>("bool", CfgType::MAIN |                                         CfgType::SCRAPER )},
        {"cacheResize",           QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )},
        {"cacheScreenshots",      QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )},
        {"cacheTextures",         QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )},
        {"cacheWheels",           QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )},
        {"cropBlack",             QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"emulator",              QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"endAt",                 QPair<QString, int>("str",                  CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"excludeFiles",          QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"excludeFrom",           QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"excludePattern",        QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"extensions",            QPair<QString, int>("str",                  CfgType::PLATFORM                                        )},
        {"forceFilename",         QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"frontend",              QPair<QString, int>("str",  CfgType::MAIN                                                            )},
        {"gameListBackup",        QPair<QString, int>("bool", CfgType::MAIN |                     CfgType::FRONTEND                    )},
        {"gamelistFolder",        QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"gameListFolder",        QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"hints",                 QPair<QString, int>("bool", CfgType::MAIN                                                            )},
        {"importFolder",          QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"includeFiles",          QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"includeFrom",           QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"includePattern",        QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"inputFolder",           QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"interactive",           QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )},
        {"jpgQuality",            QPair<QString, int>("int",  CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )},
        {"lang",                  QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"langPrios",             QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"launch",                QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"maxFails",              QPair<QString, int>("int",  CfgType::MAIN                                                            )},
        {"maxLength",             QPair<QString, int>("int",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND | CfgType::SCRAPER )},
        {"mediaFolder",           QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"mediaFolderHidden",     QPair<QString, int>("bool",                                     CfgType::FRONTEND                    )},
        {"minMatch",              QPair<QString, int>("int",  CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )},
        {"nameTemplate",          QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"platform",              QPair<QString, int>("str",  CfgType::MAIN                                                            )},
        {"pretend",               QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"region",                QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"regionPrios",           QPair<QString, int>("str",  CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"relativePaths",         QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"scummIni",              QPair<QString, int>("str",  CfgType::MAIN                                                            )},
        {"skipped",               QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"spaceCheck",            QPair<QString, int>("bool", CfgType::MAIN                                                            )},
        {"startAt",               QPair<QString, int>("str",                  CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"subdirs",               QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"symlink",               QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"theInFront",            QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"threads",               QPair<QString, int>("int",  CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )},
        {"tidyDesc",              QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )},
        {"unattend",              QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND | CfgType::SCRAPER )},
        {"unattendSkip",          QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND | CfgType::SCRAPER )},
        {"unpack",                QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM                                        )},
        {"userCreds",             QPair<QString, int>("str",                                                          CfgType::SCRAPER )},
        {"verbosity",             QPair<QString, int>("int",  CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND                    )},
        {"videoConvertCommand",   QPair<QString, int>("str",  CfgType::MAIN |                                         CfgType::SCRAPER )},
        {"videoConvertExtension", QPair<QString, int>("str",  CfgType::MAIN |                                         CfgType::SCRAPER )},
        {"videoPreferNormalized", QPair<QString, int>("bool",                                                         CfgType::SCRAPER )},
        {"videos",                QPair<QString, int>("bool", CfgType::MAIN | CfgType::PLATFORM | CfgType::FRONTEND | CfgType::SCRAPER )},
        {"videoSizeLimit",        QPair<QString, int>("int",  CfgType::MAIN | CfgType::PLATFORM |                     CfgType::SCRAPER )}
        // clang-format on
    };
};

#endif // SETTINGS_H
