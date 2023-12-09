/***************************************************************************
 *            emulationstation.cpp
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

#include "emulationstation.h"

#include "gameentry.h"
#include "platform.h"
#include "strtools.h"
#include "xmlreader.h"

#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QStringBuilder>

static inline QRegularExpression isoTimeRe() {
    return QRegularExpression("(^$|T[0-9]{6}$)");
}

EmulationStation::EmulationStation() {}

bool EmulationStation::loadOldGameList(const QString &gameListFileString) {
    // Load old game list entries so we can preserve metadata later when
    // assembling xml
    XmlReader gameListReader;
    if (gameListReader.setFile(gameListFileString)) {
        oldEntries = gameListReader.getEntries(config->inputFolder);
        return true;
    }

    return false;
}

bool EmulationStation::skipExisting(QList<GameEntry> &gameEntries,
                                    QSharedPointer<Queue> queue) {
    gameEntries = oldEntries;

    printf("Resolving missing entries...");
    int dots = 0;
    for (int a = 0; a < gameEntries.length(); ++a) {
        dots++;
        if (dots % 100 == 0) {
            printf(".");
            fflush(stdout);
        }
        QFileInfo current(gameEntries.at(a).path);
        for (int b = 0; b < queue->length(); ++b) {
            if (current.isFile()) {
                if (current.fileName() == queue->at(b).fileName()) {
                    queue->removeAt(b);
                    // We assume filename is unique, so break after getting
                    // first hit
                    break;
                }
            } else if (current.isDir()) {
                // Use current.absoluteFilePath here since it is already a path.
                // Otherwise it will use the parent folder
                if (current.absoluteFilePath() == queue->at(b).absolutePath()) {
                    queue->removeAt(b);
                    // We assume filename is unique, so break after getting
                    // first hit
                    break;
                }
            }
        }
    }
    return true;
}

void EmulationStation::preserveFromOld(GameEntry &entry) {
    for (const auto &oldEntry : oldEntries) {
        QString oldFileName = oldEntry.path.mid(oldEntry.path.lastIndexOf("/"),
                                                oldEntry.path.length());
        QString fileName =
            entry.path.mid(entry.path.lastIndexOf("/"), entry.path.length());
        if (oldFileName == fileName) {
            if (entry.eSFavorite.isEmpty()) {
                entry.eSFavorite = oldEntry.eSFavorite;
            }
            if (entry.eSHidden.isEmpty()) {
                entry.eSHidden = oldEntry.eSHidden;
            }
            if (entry.eSPlayCount.isEmpty()) {
                entry.eSPlayCount = oldEntry.eSPlayCount;
            }
            if (entry.eSLastPlayed.isEmpty()) {
                entry.eSLastPlayed = oldEntry.eSLastPlayed;
            }
            if (entry.eSKidGame.isEmpty()) {
                entry.eSKidGame = oldEntry.eSKidGame;
            }
            if (entry.eSSortName.isEmpty()) {
                entry.eSSortName = oldEntry.eSSortName;
            }
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
            break;
        }
    }
}

void EmulationStation::assembleList(QString &finalOutput,
                                    QList<GameEntry> &gameEntries) {
    for (auto &entry : gameEntries) {
        if (entry.isFolder) {
            continue;
        }
        if (config->platform == "daphne") {
            // 'daphne/roms/yadda_yadda.zip' -> 'daphne/yadda_yadda.daphne'
            entry.path.replace("daphne/roms/", "daphne/")
                .replace(".zip", ".daphne");
            continue;
        }
        QFileInfo entryInfo(entry.path);
        // Check if game is in subfolder. If so, change entry to <folder>
        // type.
        QString entryAbsolutePath = entryInfo.absolutePath();
        // Check if path is exactly one subfolder beneath root platform
        // folder (has one more '/')
        if (entryAbsolutePath.count("/") ==
            config->inputFolder.count("/") + 1) {
            QString extensions = Platform::get().getFormats(
                config->platform, config->extensions, config->addExtensions);
            // Check if the platform has both cue and bin extensions. Remove
            // bin if it does to avoid count() below to be 2. I thought
            // about removing bin extensions entirely from platform.cpp, but
            // I assume I've added them per user request at some point.
            if (extensions.contains("*.cue") && extensions.contains("*.bin")) {
                extensions.replace("*.bin", "");
                extensions = extensions.simplified();
            }
            // Check if subfolder has more roms than one, in which case we
            // stick with <game>
            if (QDir(entryAbsolutePath, extensions).count() == 1) {
                entry.isFolder = true;
                entry.path = entryAbsolutePath;
            }
        }

        // check if <folder> element(s) are needed
        QString entryAbsDir = entryInfo.absoluteDir().absolutePath();
        // config->inputFolder is absolute
        QString subPath =
            QDir(config->inputFolder).relativeFilePath(entryAbsDir);
        if (subPath != ".") {
            addFolder(config->inputFolder, subPath, gameEntries);
        }
    }

    int dots = -1;
    // Always make dotMod at least 1 or it will give "floating point exception"
    // when modulo
    int dotMod = 1 + gameEntries.length() * 0.1;

    finalOutput.append("<?xml version=\"1.0\"?>\n");
    finalOutput.append("<gameList>\n");
    for (auto &entry : gameEntries) {
        if (++dots % dotMod == 0) {
            printf(".");
            fflush(stdout);
        }
        // Preserve certain data from old game list entry, but only for empty
        // data
        preserveFromOld(entry);

        if (config->relativePaths) {
            entry.path.replace(config->inputFolder, ".");
        }
        finalOutput.append(createXml(entry));
    }
    finalOutput.append("</gameList>\n");
}

void EmulationStation::addFolder(QString &base, QString sub,
                                 QList<GameEntry> &gameEntries) {
    bool found = false;
    QString absPath = base % "/" % sub;

    for (auto entry : gameEntries) {
        if (entry.isFolder && entry.path == absPath) {
            found = true;
            break;
        }
    }

    if (!found) {
        GameEntry fe;
        fe.path = absPath;
        fe.title = sub.right(sub.lastIndexOf('/'));
        fe.isFolder = true;
        qDebug() << "addFolder() folder, path: " << fe.path;
        gameEntries.append(fe);
    }

    if (sub.contains('/')) {
        // one folder up
        sub = sub.left(sub.lastIndexOf('/'));
        addFolder(base, sub, gameEntries);
    }
}

QString EmulationStation::createXml(GameEntry &entry) {
    QStringList l;
    bool addEmptyElem = !entry.isFolder;

    QString entryType = QString(entry.isFolder ? "folder" : "game");
    l.append("  <" % entryType % ">");

    l.append(elem("path", entry.path, addEmptyElem));
    l.append(elem("name", entry.title, addEmptyElem));

    l.append(elem("thumbnail", entry.coverFile, addEmptyElem, true));
    l.append(elem("image", entry.screenshotFile, addEmptyElem, true));
    l.append(elem("marquee", entry.marqueeFile, addEmptyElem, true));
    l.append(elem("texture", entry.textureFile, addEmptyElem, true));

    QString vidFile = entry.videoFile;
    if (!config->videos) {
        vidFile = "";
    }
    l.append(elem("video", vidFile, addEmptyElem, true));

    l.append(elem("rating", entry.rating, addEmptyElem));
    l.append(elem("desc", entry.description, addEmptyElem));

    QString released = entry.releaseDate;
    QRegularExpressionMatch m = isoTimeRe().match(released);
    if (!m.hasMatch()) {
        released = released % "T000000";
    }
    l.append(elem("releasedate", released, addEmptyElem));

    l.append(elem("developer", entry.developer, addEmptyElem));
    l.append(elem("publisher", entry.publisher, addEmptyElem));
    l.append(elem("genre", entry.tags, addEmptyElem));
    l.append(elem("players", entry.players, addEmptyElem));

    // non scraper elements
    l.append(elem("sortname", entry.eSSortName, false));
    l.append(elem("favorite", entry.eSFavorite, false));
    l.append(elem("hidden", entry.eSHidden, false));
    l.append(elem("lastplayed", entry.eSLastPlayed, false));
    l.append(elem("playcount", entry.eSPlayCount, false));

    QString kidGame = entry.eSKidGame;
    if (entry.eSKidGame.isEmpty() && entry.ages.toInt() >= 1 &&
        entry.ages.toInt() <= 10) {
        kidGame = "true";
    }
    l.append(elem("kidgame", kidGame, false));

    l.append("  </" % entryType % ">");
    l.removeAll("");
    return l.join("\n") % "\n";
}

QString EmulationStation::elem(QString elem, QString &data, bool addEmptyElem,
                               bool isPath) {
    QString e;
    if (data.isEmpty()) {
        if (addEmptyElem) {
            e = QString("    <%1 />").arg(elem);
        }
    } else {
        QString d = data;
        if (isPath && config->relativePaths) {
            // The replace here IS supposed to be 'inputFolder' and not
            // 'mediaFolder' because we only want the path to be relative if
            // '-o' hasn't been set. So this will only make it relative if the
            // path is equal to inputFolder which is what we want.
            d = d.replace(config->inputFolder, ".");
        }
        d = StrTools::xmlEscape(d);
        e = QString("    <%1>%2</%1>").arg(elem, d);
    }
    return e;
}

bool EmulationStation::canSkip() { return true; }

QString EmulationStation::getGameListFileName() {
    return QString("gamelist.xml");
}

QString EmulationStation::getInputFolder() {
    return QString(QDir::homePath() % "/RetroPie/roms/" % config->platform);
}

QString EmulationStation::getGameListFolder() { return config->inputFolder; }

QString EmulationStation::getCoversFolder() {
    return config->mediaFolder % "/covers";
}

QString EmulationStation::getScreenshotsFolder() {
    return config->mediaFolder % "/screenshots";
}

QString EmulationStation::getWheelsFolder() {
    return config->mediaFolder % "/wheels";
}

QString EmulationStation::getMarqueesFolder() {
    return config->mediaFolder % "/marquees";
}

QString EmulationStation::getTexturesFolder() {
    return config->mediaFolder % "/textures";
}

QString EmulationStation::getVideosFolder() {
    return config->mediaFolder % "/videos";
}
