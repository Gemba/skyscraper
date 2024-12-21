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
#include <QStringBuilder>

EmulationStation::EmulationStation() {}

bool EmulationStation::loadOldGameList(const QString &gameListFileString) {
    // Load old game list entries so we can preserve metadata later when
    // assembling xml
    XmlReader gameListReader;
    if (gameListReader.setFile(gameListFileString)) {
        oldEntries = gameListReader.getEntries(config->inputFolder,
                                               extraGamelistTags(true));
        return true;
    }
    return false;
}

QStringList EmulationStation::extraGamelistTags(bool isFolder) {
    (void)isFolder;
    GameEntry g;
    return g.extraTagNames(GameEntry::Format::RETROPIE);
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
                // Use current.canonicalFilePath here since it is already a
                // path. Otherwise it will use the parent folder
                if (current.canonicalFilePath() ==
                    queue->at(b).canonicalPath()) {
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
        if (entry.path == oldEntry.path) {
            for (const auto &t : extraGamelistTags(entry.isFolder)) {
                if (entry.getEsExtra(t).isEmpty()) {
                    entry.setEsExtra(t, oldEntry.getEsExtra(t));
                }
            }
            if (entry.developer.isEmpty() || entry.isFolder) {
                entry.developer = oldEntry.developer;
            }
            if (entry.publisher.isEmpty() || entry.isFolder) {
                entry.publisher = oldEntry.publisher;
            }
            if (entry.players.isEmpty() || entry.isFolder) {
                entry.players = oldEntry.players;
            }
            if (entry.description.isEmpty() || entry.isFolder) {
                entry.description = oldEntry.description;
            }
            if (entry.rating.isEmpty() || entry.isFolder) {
                entry.rating = oldEntry.rating;
            }
            if (entry.releaseDate.isEmpty() || entry.isFolder) {
                entry.releaseDate = oldEntry.releaseDate;
            }
            if (entry.tags.isEmpty() || entry.isFolder) {
                entry.tags = oldEntry.tags;
            }
            if (entry.isFolder) {
                entry.title = oldEntry.title;
                entry.coverFile = oldEntry.coverFile;
                entry.screenshotFile = oldEntry.screenshotFile;
                entry.wheelFile = oldEntry.wheelFile;
                entry.marqueeFile = oldEntry.marqueeFile;
                entry.textureFile = oldEntry.textureFile;
                entry.videoFile = oldEntry.videoFile;
                // entry.manualFile on type folder does not make sense
            }
            break;
        }
    }
}

bool EmulationStation::existingInGamelist(GameEntry &entry) {
    for (const auto &oldEntry : oldEntries) {
        if (entry.path == oldEntry.path) {
            return true;
        }
    }
    return false;
}

void EmulationStation::assembleList(QString &finalOutput,
                                    QList<GameEntry> &gameEntries) {
    QString extensions = platformFileExtensions();
    // Check if the platform has both cue and bin extensions. Remove
    // bin if it does to avoid count() below to be 2. I thought
    // about removing bin extensions entirely from platform.cpp, but
    // I assume I've added them per user request at some point.
    bool cueSuffix = false;
    if (extensions.contains("*.cue")) {
        cueSuffix = true;
        if (extensions.contains("*.bin")) {
            extensions.replace("*.bin", "");
            extensions = extensions.simplified();
        }
    }

    QList<GameEntry> added;
    QDir inputDir = QDir(config->inputFolder);

    for (auto &entry : gameEntries) {
        if (config->platform == "daphne") {
            // 'daphne/roms/yadda_yadda.zip' -> 'daphne/yadda_yadda.daphne'
            entry.path.replace("daphne/roms/", "daphne/")
                .replace(".zip", ".daphne");
            continue;
        }
        if (config->platform == "scummvm") {
            // entry.path is file folder on fs with valid extension -> keep as
            // game entry
            // RetroPie/roms/scummvm/blarf.svm/ -> as <game/>
            QFileInfo entryInfo(entry.path);
            if (entryInfo.isDir() &&
                extensions.contains("*." % entryInfo.suffix().toLower())) {
                qDebug()
                    << entry.path
                    << "marked as <game/> albeit being a filesystem folder";
                continue;
            }
        }
        QFileInfo entryInfo(entry.path);
        // always use absolute file path to ROM
        entry.path = entryInfo.absoluteFilePath();

        // Check if path is exactly one subfolder beneath root platform
        // folder (has one more '/') and uses *.cue suffix
        QString entryDir = entryInfo.absolutePath();
        if (cueSuffix &&
            entryDir.count("/") == config->inputFolder.count("/") + 1) {
            // Check if subfolder has exactly one ROM, in which case we
            // use <folder>
            if (QDir(entryDir, extensions).count() == 1) {
                entry.isFolder = true;
                entry.path = entryDir;
            }
        }

        // inputDir is absolute (cf. Skyscraper::run())
        QString subPath = inputDir.relativeFilePath(entryDir);
        if (subPath != ".") {
            // <folder> element(s) are needed
            addFolder(config->inputFolder, subPath, added);
        }
    }

    gameEntries.append(added);

    int dots = -1;
    int dotMod = 1 + gameEntries.length() * 0.1;

    finalOutput.append("<?xml version=\"1.0\"?>\n");
    finalOutput.append("<gameList>\n");

    for (auto &entry : gameEntries) {
        if (++dots % dotMod == 0) {
            printf(".");
            fflush(stdout);
        }

        if (entry.isFolder && !config->addFolders &&
            !existingInGamelist(entry)) {
            qDebug() << "addFolders is false, directory not added (but may be "
                        "preserved): "
                     << entry.path;
            continue;
        }

        preserveFromOld(entry);

        if (config->relativePaths) {
            entry.path.replace(config->inputFolder, ".");
        }
        finalOutput.append(createXml(entry));
    }
    finalOutput.append("</gameList>\n");
}

void EmulationStation::addFolder(QString &base, QString sub,
                                 QList<GameEntry> &added) {
    bool found = false;
    QString absPath = base % "/" % sub;

    /*
     RetroPie/roms/scummvm/blarf.svm/blarf.svm -> leaf (fs-file) is <game/> and

     RetroPie/roms/scummvm/blarf.svm/blarf.svm -> parent fs-folder also <game/>

     RetroPie/roms/scummvm/blarf.svm/yadda/blarf.svm -> yadda as
       <folder/>, blarf.svm (fs-file and fs-folder) both as <game/>
    */

    for (auto &entry : added) {
        // check the to-be-added folder entries
        if (entry.path == absPath) {
            found = true;
            break;
        }
    }

    if (!found && !isGameLauncher(sub)) {
        GameEntry fe;
        fe.path = absPath;
        fe.title = sub.mid(sub.lastIndexOf('/') + 1, sub.length());
        fe.isFolder = true;
        qDebug() << "addFolder() adding folder elem, path:" << fe.path
                 << "with title/name:" << fe.title << ", addFolders flag is"
                 << config->addFolders;
        added.append(fe);
    }

    if (sub.contains('/')) {
        // one folder up
        sub = sub.left(sub.lastIndexOf('/'));
        addFolder(base, sub, added);
    }
}

bool EmulationStation::isGameLauncher(QString &sub) {
    bool folderIsGameLauncher = false;
    if (config->platform == "scummvm") {
        QStringList exts = platformFileExtensions().split(" ");
        for (auto ext : exts) {
            QRegExp re(ext);
            re.setPatternSyntax(QRegExp::Wildcard);
            if (re.exactMatch(sub.toLower())) {
                qDebug() << "Match: " << sub;
                // do not add if .svm or other extension is used in
                // fs-foldername
                folderIsGameLauncher = true;
                break;
            }
        }
    }
    return folderIsGameLauncher;
}

QString EmulationStation::createXml(GameEntry &entry) {
    QStringList l;
    bool addEmptyElem;
    if (gamelistFormat() == GameEntry::Format::ESDE) {
        addEmptyElem = false;
    } else {
        addEmptyElem = !entry.isFolder;
    }
    QString entryType = QString(entry.isFolder ? "folder" : "game");
    l.append("  <" % entryType % ">");

    l.append(elem("path", entry.path, addEmptyElem));
    l.append(elem("name", entry.title, addEmptyElem));

    l += createEsVariantXml(entry);

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
    for (const auto &t : extraGamelistTags(entry.isFolder)) {
        if (t != "kidgame") {
            l.append(elem(t, entry.getEsExtra(t), false));
        }
    }
    QString kidGame = entry.getEsExtra("kidgame");
    if (kidGame.isEmpty() && entry.ages.toInt() >= 1 &&
        entry.ages.toInt() <= 10) {
        kidGame = "true";
    }

    l.append(elem("kidgame", kidGame, false));

    l.append("  </" % entryType % ">");
    l.removeAll("");

    return l.join("\n") % "\n";
}

QString EmulationStation::elem(const QString &elem, const QString &data,
                               bool addEmptyElem, bool isPath) {
    QString e;
    if (data.isEmpty()) {
        if (addEmptyElem) {
            e = QString("    <%1/>").arg(elem);
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

QStringList EmulationStation::createEsVariantXml(const GameEntry &entry) {
    QStringList l;
    bool addEmptyElem = !entry.isFolder;
    l.append(elem("thumbnail", entry.coverFile, addEmptyElem, true));
    l.append(elem("image", entry.screenshotFile, addEmptyElem, true));
    l.append(elem("marquee", entry.marqueeFile, addEmptyElem, true));
    l.append(elem("texture", entry.textureFile, addEmptyElem, true));

    QString vidFile = entry.videoFile;
    if (!config->videos) {
        vidFile = "";
    }
    l.append(elem("video", vidFile, addEmptyElem, true));

    if (config->manuals &&
        config->gameListVariants.contains("enable-manuals") &&
        !entry.manualSrc.isEmpty()) {
        l.append(elem("manual", entry.manualFile, false, true));
    }
    return l;
}

bool EmulationStation::canSkip() { return true; }

QString EmulationStation::getGameListFileName() {
    return config->gameListFilename.isEmpty() ? QString("gamelist.xml")
                                              : config->gameListFilename;
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

QString EmulationStation::getManualsFolder() {
    return config->mediaFolder % "/manuals";
}
