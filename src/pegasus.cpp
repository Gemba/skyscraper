/***************************************************************************
 *            pegasus.cpp
 *
 *  Wed Jun 18 12:00:00 CEST 2017
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

#include "pegasus.h"

#include "config.h"
#include "nametools.h"
#include "strtools.h"

#include <QDate>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QTextStream>

static const QString PAD = "  ";
static const QRegularExpression RE_DOUBLE_NL =
    QRegularExpression("\\n[\\t ]*\\n");

Pegasus::Pegasus() {}

// Load old game list entries so we can preserve metadata later when
// assembling new game list
bool Pegasus::loadOldGameList(const QString &gameListFileString) {

    QFile gameListFile(gameListFileString);
    if (!gameListFile.exists() || !gameListFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QTextStream in(&gameListFile);
    QString *multiline = nullptr;
    GameEntry *currEntry = nullptr;

    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.trimmed().isEmpty() || line.startsWith('#')) {
            continue;
        } else if (line.startsWith(' ') || line.startsWith('\t')) { // multiline
            if (multiline) {
                multiline->append("\n" + line.trimmed());
            }
            continue;
        } else if (line.startsWith(':') ||
                   !line.contains(':')) { // unknown line format
            continue;
        }

        multiline = nullptr;

        QString key = line.section(':', 0, 0).trimmed();
        QString value = line.section(':', 1).trimmed();

        if (key == "game") {
            GameEntry newEntry;
            newEntry.title = value;
            oldEntries.append(newEntry);
            currEntry = &oldEntries.last();
        } else if (!currEntry) { // This is a header line
            if (key == "command") {
                key = "launch"; // "command" is an alias of "launch"
            } else if (key == "cwd") {
                key = "workdir"; // "cwd" is an alias of "workdir"
            }
            multiline = &headerPairs.insert(key, value).value();
        } else if (key == "file" || key == "files") {
            currEntry->path = value;
        } else if (key == "developer") {
            currEntry->developer = value;
        } else if (key == "publisher") {
            currEntry->publisher = value;
        } else if (key == "release") {
            currEntry->releaseDate =
                QDate::fromString(value, "yyyy-MM-dd").toString("yyyyMMdd");
        } else if (key == "genre") {
            currEntry->tags = value;
        } else if (key == "players") {
            currEntry->players = value;
        } else if (key == "rating") {
            currEntry->rating =
                QString::number(value.remove('%').toDouble() / 100.0);
        } else if (key == "assets.boxFront") {
            currEntry->coverFile = makeAbsolute(value, config->inputFolder);
        } else if (key == "assets.screenshot") {
            currEntry->screenshotFile =
                makeAbsolute(value, config->inputFolder);
        } else if (key == "assets.marquee") {
            currEntry->marqueeFile = makeAbsolute(value, config->inputFolder);
        } else if (key == "assets.wheel") {
            currEntry->wheelFile = makeAbsolute(value, config->inputFolder);
        } else if (key == "assets.cartridge") {
            currEntry->textureFile = makeAbsolute(value, config->inputFolder);
        } else if (key == "assets.video") {
            currEntry->videoFile = makeAbsolute(value, config->inputFolder);
            currEntry->videoFormat = value.split('.').last();
        } else if (key == "description") {
            currEntry->description = value;
            multiline = &currEntry->description;
        } else {
            currEntry->pSValuePairs.append({key, value});
            multiline = &currEntry->pSValuePairs.last().second;
        }
    }

    return true;
}

QString Pegasus::makeAbsolute(const QString &filePath,
                              const QString &inputFolder) {
    return Config::makeAbsolutePath(inputFolder, QString(filePath));
}

void Pegasus::skipExisting(QList<GameEntry> &gameEntries,
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
                    // We assume filename is unique, so break after getting
                    // first hit
                    break;
                }
            } else if (current.isDir()) {
                // Use current.absoluteFilePath here since it is already a
                // path. Otherwise it will use the parent folder
                if (current.absoluteFilePath() == (*qi).absoluteFilePath()) {
                    queue->erase(qi);
                    // We assume filename is unique, so break after getting
                    // first hit
                    break;
                }
            }
        }
    }
}

void Pegasus::preserveFromOld(GameEntry &entry) {
    QString fn = getFilename(entry.path);
    for (const auto &oldEntry : oldEntries) {
        if (getFilename(oldEntry.path) == fn) {
            if (entry.developer.isEmpty()) {
                entry.developer = oldEntry.developer;
            }
            if (entry.publisher.isEmpty()) {
                entry.publisher = oldEntry.publisher;
            }
            if (entry.players.isEmpty()) {
                entry.players = oldEntry.players;
            }
            if (entry.description.isEmpty()) {
                entry.description = oldEntry.description;
            }
            if (entry.rating.isEmpty()) {
                entry.rating = oldEntry.rating;
            }
            if (entry.releaseDate.isEmpty()) {
                entry.releaseDate = oldEntry.releaseDate;
            }
            if (entry.tags.isEmpty()) {
                entry.tags = oldEntry.tags;
            }
            entry.pSValuePairs = oldEntry.pSValuePairs;
            break;
        }
    }
}

QString Pegasus::fromPreservedHeader(const QString &key,
                                     const QString &suggested) {
    if (auto it = headerPairs.find(key); it != headerPairs.end()) {
        QString preserved = it.value();
        headerPairs.erase(it);
        return preserved;
    }
    return suggested;
}

void Pegasus::removePreservedHeader(const QString &key) {
    if (auto it = headerPairs.find(key); it != headerPairs.end())
        headerPairs.erase(it);
}

QString Pegasus::toPegasusFormat(const QString &key, const QString &value) {
    QString pegasusFormat = value;

    QRegularExpressionMatch match;
    match = RE_DOUBLE_NL.match(pegasusFormat);
    for (const auto &capture : match.capturedTexts()) {
        pegasusFormat.replace(capture,
                              "###NEWLINE###" % PAD % ".###NEWLINE###" % PAD);
    }
    pegasusFormat.replace("\n", "\n" % PAD);
    pegasusFormat.replace("###NEWLINE###", "\n");
    pegasusFormat = key % ": " % pegasusFormat;
    return pegasusFormat.trimmed();
}

void Pegasus::assembleList(QString &finalOutput,
                           QList<GameEntry> &gameEntries) {
    if (gameEntries.isEmpty())
        return;

    QStringList out;
    out.append("collection: " %
               fromPreservedHeader("collection", gameEntries.first().platform));
    out.append("shortname: " %
               fromPreservedHeader("shortname", config->platform));
    if (config->frontendExtra.isEmpty()) {
        out.append("launch: " %
                   fromPreservedHeader(
                       "launch", "/opt/retropie/supplementary/runcommand/"
                                 "runcommand.sh 0 _SYS_ " %
                                     config->platform % " \"{file.path}\""));
    } else {
        out.append("launch: " % config->frontendExtra);
        removePreservedHeader("launch");
    }

    for (auto it = headerPairs.begin(); it != headerPairs.end(); it++) {
        out.append(toPegasusFormat(it.key(), it.value()));
    }

    int dots = -1;
    int dotMod = gameEntries.length() * 0.1 + 1;
    const bool useRelPath = config->relativePaths;

    for (auto &entry : gameEntries) {
        if (++dots % dotMod == 0) {
            printf(".");
            fflush(stdout);
        }

        preserveFromOld(entry);

        out.append("");
        out.append(toPegasusFormat("game", entry.title));

        if (useRelPath) {
            entry.path = Config::lexicallyRelativePath(config->gameListFolder,
                                                       entry.path);
        }

        out.append(toPegasusFormat("file", entry.path));

        if (!entry.rating.isEmpty()) {
            out.append(toPegasusFormat(
                "rating",
                QString::number((int)(entry.rating.toDouble() * 100)) % "%"));
        }
        if (!entry.description.isEmpty()) {
            QString desc = entry.description.left(config->maxLength);
            replaceColon(desc, entry.title);
            out.append(toPegasusFormat("description", desc));
        }
        if (!entry.releaseDate.isEmpty()) {
            out.append(toPegasusFormat(
                "release", QDate::fromString(entry.releaseDate, "yyyyMMdd")
                               .toString("yyyy-MM-dd")));
        }
        if (!entry.developer.isEmpty()) {
            out.append(toPegasusFormat("developer", entry.developer));
        }
        if (!entry.publisher.isEmpty()) {
            out.append(toPegasusFormat("publisher", entry.publisher));
        }
        if (!entry.tags.isEmpty()) {
            out.append(toPegasusFormat("genre", entry.tags));
        }
        if (!entry.players.isEmpty()) {
            out.append(toPegasusFormat("players", entry.players));
        }
        if (!entry.screenshotFile.isEmpty()) {
            out.append(addMediaFile("assets.screenshot", useRelPath,
                                    entry.screenshotFile));
        }
        if (!entry.coverFile.isEmpty()) {
            out.append(
                addMediaFile("assets.boxFront", useRelPath, entry.coverFile));
        }
        if (!entry.marqueeFile.isEmpty()) {
            out.append(
                addMediaFile("assets.marquee", useRelPath, entry.marqueeFile));
        }
        if (!entry.textureFile.isEmpty()) {
            out.append(addMediaFile("assets.cartridge", useRelPath,
                                    entry.textureFile));
        }
        if (!entry.wheelFile.isEmpty()) {
            out.append(
                addMediaFile("assets.wheel", useRelPath, entry.wheelFile));
        }
        if (!entry.videoFormat.isEmpty() && config->videos) {
            out.append(
                addMediaFile("assets.video", useRelPath, entry.videoFile));
        }
        if (!entry.pSValuePairs.isEmpty()) {
            for (const auto &pair : entry.pSValuePairs) {
                out.append(toPegasusFormat(pair.first, pair.second));
            }
        }
    }
    finalOutput = out.join("\n") + "\n";
}

QString Pegasus::addMediaFile(const QString &asset, bool useRelativePath,
                              QString mediaFile) {
    if (useRelativePath) {
        mediaFile =
            Config::lexicallyRelativePath(config->gameListFolder, mediaFile);
    }
    return toPegasusFormat(asset, mediaFile);
}

void Pegasus::replaceColon(QString &value, const QString &gameTitle) {
    int idx = value.indexOf(':');
    while (idx != -1) {
        QString ctxStr;
        int begin = 0;
        int end = value.length();
        if (idx - 12 > 0) {
            ctxStr = "...";
            begin = idx - 12;
        }
        QString endStr = "";
        if (idx + 13 < end) {
            end = idx + 13;
            endStr = "...";
        }
        // TODO: use sliced() instead of mid() when Qt5
        // is no longer supported
        ctxStr = ctxStr % value.mid(begin, end - begin) % endStr;
        value.replace(idx, 1, ".");
        qWarning() << QString(
                          "Description of '%1' contains a colon (:) at '%2', "
                          "Skyscraper replaced it with '.'. Consider "
                          "editing the description to remediate this warning.")
                          .arg(gameTitle)
                          .arg(ctxStr);
        idx = value.indexOf(':');
    }
}

bool Pegasus::canSkip() { return true; }

QString Pegasus::getGameListFileName() {
    return config->gameListFilename.isEmpty() ? QString("metadata.pegasus.txt")
                                              : config->gameListFilename;
}

QString Pegasus::getInputFolder() {
    return QString(QDir::homePath() % "/RetroPie/roms/" % config->platform);
}

QString Pegasus::getGameListFolder() {
    return QString(QDir::homePath() % "/RetroPie/roms/" % config->platform);
}

QString Pegasus::getCoversFolder() { return config->mediaFolder % "/covers"; }

QString Pegasus::getScreenshotsFolder() {
    return config->mediaFolder % "/screenshots";
}

QString Pegasus::getWheelsFolder() { return config->mediaFolder % "/wheels"; }

QString Pegasus::getMarqueesFolder() {
    return config->mediaFolder % "/marquees";
}

QString Pegasus::getTexturesFolder() {
    return config->mediaFolder % "/textures";
}

QString Pegasus::getVideosFolder() { return config->mediaFolder % "/videos"; }
