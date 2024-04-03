/*
 *  This file is part of skyscraper.
 *  Copyright 2017 Lars Muldjord
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

#include "config.h"

#include "platform.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>

void Config::copyFile(const QString &src, const QString &dest, FileOp fileOp) {
    if (QFileInfo::exists(src)) {
        if (QFileInfo::exists(dest)) {
            if (fileOp == FileOp::OVERWRITE) {
                QFile::remove(dest);
                QFile::copy(src, dest);
                qDebug() << "Overwritten file" << dest;
            } else if (fileOp == FileOp::CREATE_DIST) {
                QString d = QString(dest + ".dist");
                QFile::remove(d);
                QFile::copy(src, d);
                qDebug() << "Created original dist file as" << d;
            }
        } else {
            QFile::copy(src, dest);
            qDebug() << "Created file" << dest;
        }
    } else {
        printf("\033[1;31mSource config file not found '%s'. Please check "
               "setup, bailing out...\033[0m\n",
               src.toStdString().c_str());
        exit(1);
    }
}

void Config::setupUserConfig() {
    // Set the working directory to the applications own path
    QDir skyDir(QDir::homePath() + "/.skyscraper");
    if (!skyDir.exists()) {
        if (!skyDir.mkpath(".")) {
            printf("Couldn't create folder '%s'. Please check permissions, "
                   "now exiting...\n",
                   skyDir.absolutePath().toStdString().c_str());
            exit(1);
        }
    }

    // Create import paths
    skyDir.mkpath("import/textual");
    skyDir.mkpath("import/screenshots");
    skyDir.mkpath("import/covers");
    skyDir.mkpath("import/wheels");
    skyDir.mkpath("import/marquees");
    skyDir.mkpath("import/textures");
    skyDir.mkpath("import/videos");

    // Create resources folder
    skyDir.mkpath("resources");

    // Rename 'dbs' folder to migrate 2.x users to 3.x
    skyDir.rename(skyDir.absolutePath() + "/dbs",
                  skyDir.absolutePath() + "/cache");

    // Create cache folder
    skyDir.mkpath("cache");

    QDir::setCurrent(skyDir.absolutePath());

    // copy configs
    QString localEtcPath = QString(PREFIX "/etc/skyscraper/");

    if (!QFileInfo::exists(localEtcPath)) {
        // RetroPie or Windows installation type: handled externally
        return;
    }

    QMap<QString, QPair<QString, FileOp>> configFiles = {
        {"ARTWORK.md", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"artwork.xml.example1", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"artwork.xml.example2", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"artwork.xml.example3", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"artwork.xml.example4", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"cache/priorities.xml.example",
         QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"config.ini.example",
         QPair<QString, FileOp>("config.ini.example", FileOp::OVERWRITE)},
        {"CACHE.md",
         QPair<QString, FileOp>("cache/README.md", FileOp::OVERWRITE)},
        {"hints.xml", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"import/definitions.dat.example1",
         QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"import/definitions.dat.example2",
         QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"import/IMPORT.md",
         QPair<QString, FileOp>("import/README.md", FileOp::OVERWRITE)},
        {"mameMap.csv", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"mobygames_platforms.json",
         QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"README.md", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"resources/boxfront.png",
         QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"resources/boxside.png",
         QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"screenscraper_platforms.json",
         QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"tgdb_developers.json", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"tgdb_genres.json", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"tgdb_platforms.json", QPair<QString, FileOp>("", FileOp::OVERWRITE)},
        {"tgdb_publishers.json", QPair<QString, FileOp>("", FileOp::OVERWRITE)},

        {"config.ini.example",
         QPair<QString, FileOp>("config.ini", FileOp::KEEP)},
        {"import/definitions.dat.example2",
         QPair<QString, FileOp>("import/definitions.dat", FileOp::KEEP)},
        {"resources/frameexample.png",
         QPair<QString, FileOp>("", FileOp::KEEP)},
        {"resources/maskexample.png", QPair<QString, FileOp>("", FileOp::KEEP)},
        {"resources/scanlines1.png", QPair<QString, FileOp>("", FileOp::KEEP)},
        {"resources/scanlines2.png", QPair<QString, FileOp>("", FileOp::KEEP)},
        // create <fn>.dist if exists
        {"aliasMap.csv", QPair<QString, FileOp>("", FileOp::CREATE_DIST)},
        {"artwork.xml", QPair<QString, FileOp>("", FileOp::CREATE_DIST)},
        {"peas.json", QPair<QString, FileOp>("", FileOp::CREATE_DIST)},
        {"platforms_idmap.csv",
         QPair<QString, FileOp>("", FileOp::CREATE_DIST)}
    };

    for (auto src : configFiles.keys()) {
        QString dest = configFiles.value(src).first;
        if (dest.isEmpty()) {
            dest = src;
        }
        copyFile(localEtcPath + src, dest, configFiles.value(src).second);
    }
}

void Config::checkLegacyFiles() {
    QStringList legacyJsons =
        QString("mobygames platforms screenscraper").split(" ");
    for (auto bn : legacyJsons) {
        QString fn = bn + ".json";
        if (QFileInfo::exists(fn)) {
            printf(
                "\033[1;33mFile '%s' found, which is no longer used in this "
                "version of Skyscraper. Please move file to mute this warning. "
                "See docs/PLATFORMS.md for additional info.\033[0m\n",
                fn.toUtf8().constData());
        }
    }
}

QString Config::getSupportedPlatforms() {
    if (!Platform::get().loadConfig()) {
        exit(1);
    }

    QString platforms;
    for (const auto &platform : Platform::get().getPlatforms()) {
        platforms.append("'" + platform + "', ");
    }
    platforms.chop(2);
    return platforms;
}
