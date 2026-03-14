/*
 *  This file is part of skyscraper.
 *  Copyright 2026 SineSwiper @ GitHub
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

#include "retroarch.h"

#include "gameentry.h"
#include "pathtools.h"
#include "platform.h"
#include "strtools.h"

#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QTextStream>

RetroArch::RetroArch() {}

QString RetroArch::sanitizeForFilename(const QString &name) {
    QString sanitized = name;
    // Replace forbidden characters with underscore
    sanitized.replace(QRegularExpression("[&*/:\\\\\"<>?|]"), "_");
    return sanitized;
}

QString RetroArch::jsonEscape(const QString &str) {
    QString escaped = str;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    escaped.replace("\n", "\\n");
    escaped.replace("\r", "\\r");
    escaped.replace("\t", "\\t");
    return escaped;
}

const QString RetroArch::getPlatformOutputName() {
    // Look up the RetroArch db_name from platforms_idmap.csv.
    QString dbName = Platform::get().getRetroArchDbName(config->platform);

    if (dbName.isEmpty()) {
        // Fallback to platform name if not found
        qWarning() << "Platform" << config->platform
                   << "not in RetroArch platform mapping, using platform name "
                      "as-is";
        dbName = config->platform;
    }
    return dbName;
}

bool RetroArch::loadOldGameList(const QString &gameListFileString) {
    QFile gameListFile(gameListFileString);
    if (!gameListFile.exists() || !gameListFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray jsonData = gameListFile.readAll();
    gameListFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray items = root.value("items").toArray();

    for (const QJsonValue &item : items) {
        if (!item.isObject()) {
            continue;
        }

        QJsonObject itemObj = item.toObject();
        GameEntry newEntry;
        newEntry.path = itemObj.value("path").toString();
        newEntry.title = itemObj.value("label").toString();
        oldEntries.append(newEntry);
    }

    return true;
}

void RetroArch::skipExisting(QList<GameEntry> &gameEntries,
                             QSharedPointer<Queue> queue) {
    gameEntries = oldEntries;

    printf("Resolving missing entries...");
    int dots = 0;
    for (auto const &ge : gameEntries) {
        dots++;
        if (dots % 100 == 0) {
            printf(".");
            fflush(stdout);
        }
        QFileInfo current(ge.path);
        for (auto qi = queue->begin(), end = queue->end(); qi != end; ++qi) {
            if (current.isFile()) {
                if (current.fileName() == (*qi).fileName()) {
                    queue->erase(qi);
                    break;
                }
            } else if (current.isDir()) {
                if (current.absoluteFilePath() == (*qi).absoluteFilePath()) {
                    queue->erase(qi);
                    break;
                }
            }
        }
    }
}

void RetroArch::preserveFromOld(GameEntry &entry) {
    QString fn = entry.baseName;
    for (const auto &oldEntry : oldEntries) {
        if (QFileInfo(oldEntry.path).fileName() == fn) {
            if (entry.title.isEmpty()) {
                entry.title = oldEntry.title;
            }
            break;
        }
    }
}

void RetroArch::assembleList(QString &finalOutput,
                             QList<GameEntry> &gameEntries) {
    if (gameEntries.isEmpty())
        return;

    // Build a map of baseName -> title for use in getTargetFileName
    baseNameToTitle.clear();
    for (const auto &entry : gameEntries) {
        baseNameToTitle[entry.baseName] = entry.title;
    }

    QJsonObject root;
    root.insert("version", "1.5");

    // Parse default_core_path and default_core_name from frontendExtra
    // Format: "<CORE_PATH>;<CORE_NAME>" or leave as DETECT
    QString corePathStr = "DETECT";
    QString coreNameStr = "DETECT";

    if (!config->frontendExtra.isEmpty()) {
        // frontendExtra is used for default_core_path and default_core_name
        // when it contains a path separator (/). Format:
        // "<CORE_PATH>;<CORE_NAME>"
        if (config->frontendExtra.contains("/")) {
            QStringList parts = config->frontendExtra.split(";");
            if (parts.length() >= 1) {
                corePathStr = parts[0];
            }
            if (parts.length() >= 2) {
                coreNameStr = parts[1];
            }
        }
    }

    root.insert("default_core_path", corePathStr);
    root.insert("default_core_name", coreNameStr);
    root.insert("label_display_mode", "0");   // show full labels
    root.insert("left_display_mode", "0");    // system default
    root.insert("right_display_mode", "0");   // system default
    root.insert("thumbnail_match_mode", "0"); // system default
    root.insert("sort_mode", "0");            // system default

    QJsonArray items;

    int dots = -1;
    int dotMod = gameEntries.length() * 0.1 + 1;

    for (const auto &entry : gameEntries) {
        if (++dots % dotMod == 0) {
            printf(".");
            fflush(stdout);
        }

        // TODO: unpack support for CRC and inter-zip reference
        //     "path": "/storage/emulated/0/ROMs/virtualboy/Game.zip#Game.vb",
        //     "crc32": "133E9372|crc",

        QJsonObject itemObj;
        itemObj.insert("path", entry.absoluteFilePath);
        itemObj.insert("label", entry.title);
        itemObj.insert("core_path", "DETECT");
        itemObj.insert("core_name", "DETECT");
        itemObj.insert("crc32", "DETECT");
        itemObj.insert("db_name", QString(getPlatformOutputName() + ".lpl"));

        items.append(itemObj);
    }

    root.insert("items", items);

    QJsonDocument doc(root);
    finalOutput = doc.toJson(QJsonDocument::Indented);
}

QString RetroArch::getTargetFileName(GameEntry::Types t,
                                     const QString &baseName) {
    (void)t; // Suppress unused parameter warning

    // Look up the game title from our map
    QString title = baseNameToTitle.value(baseName, baseName);

    // Sanitize the title for use as a filename
    return sanitizeForFilename(title);
}

bool RetroArch::canSkip() { return true; }

QString RetroArch::getGameListFileName() {
    return config->gameListFilename.isEmpty()
               ? (getPlatformOutputName() + ".lpl")
               : config->gameListFilename;
}

QString RetroArch::getInputFolder() {
    return QString(QDir::homePath() % "/RetroPie/roms/" % config->platform);
}

QString RetroArch::getGameListFolder() {
    return "/opt/retropie/configs/all/retroarch/playlists";
}

QString RetroArch::getMediaFolder() {
    return "/opt/retropie/configs/all/retroarch/thumbnails";
}

QString RetroArch::getCoversFolder() {
    return config->mediaFolder % "/Named_Boxarts";
}

QString RetroArch::getScreenshotsFolder() {
    return config->mediaFolder % "/Named_Snaps";
}

QString RetroArch::getMarqueesFolder() {
    return config->mediaFolder % "/Named_Logos";
}

QString RetroArch::getWheelsFolder() {
    return config->mediaFolder % "/Named_Logos";
}

// XXX: This media type does exist yet...

/*
QString RetroArch::getTitleScreenshotsFolder() {
    return config->mediaFolder % "/Named_Titles";
}
*/