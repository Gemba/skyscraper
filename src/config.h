/*
 *  This file is part of skyscraper.
 *  Copyright 2023 Gemba @ GitHub
 *
 *  skyscraper is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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

#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QObject>
#include <QString>

class Config : public QObject {
    Q_OBJECT

public:
    enum class FileOp { KEEP, OVERWRITE, CREATE_DIST };
    enum class SkyFolderType { CONFIG, CACHE, IMPORT, RESOURCE, REPORT, LOG };
    enum class Setup { MAKE, RETROPIE, WINDOWS, SINGLE_BIN };

    typedef QMap<SkyFolderType, QString> SkyFolders;

    static QString getSkyFolder(SkyFolderType type = SkyFolderType::CONFIG);
    static QString getRetropieVersion();
    static QString adjustDestinationPath(const QString &src, QString &dest);

    void initSkyFolders();
    Setup setupUserConfig();
    void checkLegacyFiles();
    void copyFile(const QString &src, const QString &dest, bool isPristine,
                  FileOp fileOp = FileOp::OVERWRITE);
    int isPlatformCfgPristine(QString platformCfgFilePath);
    QString getSupportedPlatforms();
    void configInfo();
    bool createDefaultConfigIni();
#ifdef TESTING
    QString validateConfigSrcPath(const QString &sysConfPath,
                                  const QString &prefixPath,
                                  const QString &applDirPath);
#endif

signals:
    void die(const int &, const QString &, const QString &);

private:
#ifndef TESTING
    QString validateConfigSrcPath(const QString &sysConfPath,
                                  const QString &prefixPath,
                                  const QString &applDirPath);
#endif

    // copy configs
    QMap<QString, QPair<QString, FileOp>> configFiles = {
        // clang-format off
        {"ARTWORK.md",                      QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"artwork.xml.example1",            QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"artwork.xml.example2",            QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"artwork.xml.example3",            QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"artwork.xml.example4",            QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"cache/priorities.xml.example",    QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"config.ini.example",              QPair<QString, FileOp>("config.ini.example", FileOp::OVERWRITE)},
        {"CACHE.md",                        QPair<QString, FileOp>("cache/README.md", FileOp::OVERWRITE)},
        {"hints.xml",                       QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"import/definitions.dat.example1", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"import/definitions.dat.example2", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"import/IMPORT.md",                QPair<QString, FileOp>("import/README.md", FileOp::OVERWRITE)},
        {"mameMap.csv",                     QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"mobygames_platforms.json",        QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"README.md",                       QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"resources/boxfront.png",          QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"resources/boxside.png",           QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"screenscraper_platforms.json",    QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"tgdb_developers.json",            QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"tgdb_genres.json",                QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"tgdb_platforms.json",             QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"tgdb_publishers.json",            QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        // do not overwrite
        {"config.ini.example!",              QPair<QString, FileOp>("config.ini", FileOp::KEEP)},
        {"import/definitions.dat.example2!", QPair<QString, FileOp>("import/definitions.dat", FileOp::KEEP)},
        {"resources/frameexample.png",      QPair<QString, FileOp>("", FileOp::KEEP)},
        {"resources/maskexample.png",       QPair<QString, FileOp>("", FileOp::KEEP)},
        {"resources/scanlines1.png",        QPair<QString, FileOp>("", FileOp::KEEP)},
        {"resources/scanlines2.png",        QPair<QString, FileOp>("", FileOp::KEEP)},
        // create <fn>.dist by default if exists
        {"aliasMap.csv",                    QPair<QString, FileOp>("", FileOp::CREATE_DIST)},
        {"artwork.xml",                     QPair<QString, FileOp>("", FileOp::CREATE_DIST)},
        {"batocera-artwork.xml",            QPair<QString, FileOp>("", FileOp::CREATE_DIST)},
        {"retroarch-artwork.xml",           QPair<QString, FileOp>("", FileOp::CREATE_DIST)},
        {"peas.json",                       QPair<QString, FileOp>("", FileOp::CREATE_DIST)},
        {"platforms_idmap.csv",             QPair<QString, FileOp>("", FileOp::CREATE_DIST)}
        // clang-format on
    };
    QMap<QString, QString> essentialCfgs{
        // clang-format off
        {"artwork.xml",                  "gamelist output, failsafe unless provided by user, edit to needs"},
        {"cache/priorities.xml.example", "-\"-, blueprint of prio of scrape sources, edit in cache/<platform>/"},
        {"import/definitions.dat",       "import module/scraper, failsafe unless provided by user, edit to needs"},
        {"hints.xml",                    "general, tip of the day, readonly use"},
        {"mameMap.csv",                  "arcade scraping, filename to title mapping, readonly use"},
        {"peas.json",                    "general scraping, platform definitions, readonly, edit in *_local.json"},
        {"platforms_idmap.csv",          "-\"-, hints for API-based scraper, readonly, edit in *_local.csv"},
        {"resources/boxfront.png",       "gamelist output, artwork for type gamebox, may be user provided"},
        {"resources/boxside.png",        "gamelist output, artwork for type gamebox, may be user provided"},
        {"resources/scanlines1.png",     "gamelist output, artwork to emulate scanlines, may be user provided"},
        {"tgdb_developers.json",         "TGDB scraper, additional API info, readonly use"},
        {"tgdb_genres.json",             "TGDB scraper, additional API info, readonly use"},
        {"tgdb_platforms.json",          "TGDB scraper, additional API info, readonly use"},
        {"tgdb_publishers.json",         "TGDB scraper, additional API info, readonly use"}
        // clang-format on
    };
};

#endif // CONFIG_H
