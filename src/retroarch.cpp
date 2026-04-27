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
#include "platform.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringBuilder>

static const QString LPL_VERSION_VAL = "1.5";
static const QString DETECT_VAL = "DETECT";

const QString META_VERSION = "version";
const QString META_DFLT_CORE_PATH = "default_core_path";
const QString META_DFLT_CORE_NAME = "default_core_name";

const QStringList LPL_META_PROPS = {
    META_DFLT_CORE_PATH,    META_DFLT_CORE_NAME, META_VERSION,
    "label_display_mode",   "left_display_mode", "right_display_mode",
    "thumbnail_match_mode", "sort_mode",         "base_content_directory"};

const QString ITEMS_ARRAY = "items";

const QString ITEM_CORE_NAME = "core_name";
const QString ITEM_CORE_PATH = "core_path";
const QString ITEM_CRC = "crc32";
const QString ITEM_DB_NAME = "db_name";
const QString ITEM_LABEL = "label";
const QString ITEM_PATH = "path";

RetroArch::RetroArch() {}

QString RetroArch::sanitizeForFilename(const QString &name) {
    QString sanitized = name;
    // Replace forbidden characters with underscore
    sanitized.replace(QRegularExpression("[&*/:\\\\\"<>?|]"), "_");
    return sanitized;
}

const QString RetroArch::getPlatformOutputName() {
    // Look up the RetroArch db_name from peas.json
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
    QJsonDocument doc;
    if (QFile gameListFile(gameListFileString);
        !gameListFile.open(QIODevice::ReadOnly)) {
        return false;
    } else {
        QByteArray jsonData = gameListFile.readAll();
        gameListFile.close();
        doc = QJsonDocument::fromJson(jsonData);
        if (!doc.isObject()) {
            return false;
        }
    }

    existingPlaylist = doc.object();
    QJsonArray items = existingPlaylist.value(ITEMS_ARRAY).toArray();

    for (const QJsonValue &item : items) {
        if (item.isObject()) {
            QJsonObject itemObj = item.toObject();
            GameEntry oldEntry;
            // always absolute path with Retroarch
            // path might be contain backslashes on Windows
            oldEntry.path = itemObj.value(ITEM_PATH).toString();
            oldEntry.title = itemObj.value(ITEM_LABEL).toString();
            // remaining properties of an item are held in existingPlaylist
            oldEntries.append(oldEntry);
        }
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

void RetroArch::assembleList(QString &finalOutput,
                             QList<GameEntry> &gameEntries) {
    if (gameEntries.isEmpty())
        return;

    // Build a map of baseName -> title for use in getTargetFileName
    baseNameToTitle.clear();
    for (const auto &entry : gameEntries) {
        baseNameToTitle[entry.baseName] = entry.title;
    }

    QJsonObject newPlaylist = createMetaProps();

    QJsonArray exitsingItems = existingPlaylist.value(ITEMS_ARRAY).toArray();
    QJsonObject eitemObj;

    QJsonArray items;
    QString gameFn;

    int dots = -1;
    int dotMod = gameEntries.length() * 0.1 + 1;
    for (auto const &entry : gameEntries) {
        if (++dots % dotMod == 0) {
            printf(".");
            fflush(stdout);
        }
        gameFn = QFileInfo(entry.path).fileName();
        // TODO: unpack support for CRC and inter-zip reference
        //     "path": "/storage/emulated/0/ROMs/virtualboy/Game.zip#Game.vb",
        //     "crc32": "133E9372|crc",
        QString absPath = entry.absoluteFilePath;
#ifdef Q_OS_WIN
        absPath = absPath.replace("/", "\\\\");
#endif
        bool hasExisting = false;
        QJsonObject itemObj;
        for (const QJsonValue &eit : exitsingItems) {
            if (eit.isObject()) {
                eitemObj = eit.toObject();
                if (eitemObj[ITEM_PATH].toString().endsWith(gameFn)) {
                    hasExisting = true;
                    itemObj = eitemObj;
                    break;
                }
            }
        }

        if (!hasExisting) {
            itemObj.insert(ITEM_CORE_PATH, DETECT_VAL);
            itemObj.insert(ITEM_CORE_NAME, DETECT_VAL);
            itemObj.insert(ITEM_CRC, DETECT_VAL);
            itemObj.insert(ITEM_DB_NAME, getGameListFileName());
        }

        itemObj.insert(ITEM_PATH, absPath);
        itemObj.insert(ITEM_LABEL, entry.title);

        items.append(itemObj);
    }

    newPlaylist.insert(ITEMS_ARRAY, items);

    QJsonDocument doc(newPlaylist);
    finalOutput = doc.toJson(QJsonDocument::Indented);
}

QJsonObject RetroArch::createMetaProps() {
    QJsonObject newPlaylist;
    newPlaylist.insert(META_VERSION, LPL_VERSION_VAL);

    QString corePathStr = DETECT_VAL;
    QString coreNameStr = DETECT_VAL;

    // Parse default_core_path and default_core_name from frontendExtra
    // (raExtra= or -e)
    if (!config->frontendExtra.isEmpty()) {
        QStringList parts = config->frontendExtra.split(";");
        corePathStr = parts[0];
        coreNameStr = parts[1];
    }

    // create or restore meta properties
    for (const auto &k : LPL_META_PROPS) {
        QString v = existingPlaylist[k].toString();
        if (v.isEmpty()) {
            if (k == META_VERSION)
                newPlaylist.insert(k, LPL_VERSION_VAL);
            else if (k == META_DFLT_CORE_NAME)
                newPlaylist.insert(k, coreNameStr);
            else if (k == META_DFLT_CORE_PATH)
                newPlaylist.insert(k, corePathStr);
            else if (k == "base_content_directory")
                ; // don't set default "base_content_directory"
            else
                newPlaylist.insert(k, "0");
        } else {
            newPlaylist.insert(k, v);
        }
    }
    return newPlaylist;
}

QString RetroArch::getTargetFileName(GameEntry::Types t,
                                     const QString &baseName) {
    (void)t;
    // for media files use sanitized title as filename stem
    QString title = baseNameToTitle.value(baseName, baseName);
    return sanitizeForFilename(title);
}

bool RetroArch::canSkip() { return true; }

QString RetroArch::getGameListFileName() {
    return config->gameListFilename.isEmpty()
               ? (getPlatformOutputName() % ".lpl")
               : config->gameListFilename;
}

QString RetroArch::getInputFolder() {
    return QString(QDir::homePath() % "/RetroPie/roms/" % config->platform);
}

QString RetroArch::getGameListFolder() {
    if (config->gameListFolder.isEmpty()) {
        return QDir::homePath() % "/.config/retroarch/playlists";
    } else {
        if (config->gameListFolder.endsWith("/" % config->platform)) {
            return config->gameListFolder.replace("/" % config->platform, "");
        }
        return config->gameListFolder;
    }
}

QString RetroArch::getMediaFolder() {
    if (config->mediaFolder.isEmpty()) {
        return QDir::homePath() % "/.config/retroarch/thumbnails/" %
               getPlatformOutputName();
    } else {
        if (config->mediaFolder.endsWith("/" % config->platform)) {
            return config->mediaFolder.replace("/" % config->platform,
                                               "/" % getPlatformOutputName());
        }
        return config->mediaFolder;
    }
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

// PENDING: This media type does exist yet...

/*
QString RetroArch::getTitleScreenshotsFolder() {
    return config->mediaFolder % "/Named_Titles";
}
*/
