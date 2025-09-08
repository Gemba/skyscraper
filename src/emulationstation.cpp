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

#include "config.h"
#include "gameentry.h"
#include "strtools.h"
#include "xmlreader.h"

#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QStringBuilder>

static const QRegularExpression REGEX_OPENELEM =
    QRegularExpression("<(\\w+)[\\s>]");

EmulationStation::EmulationStation() {}

void EmulationStation::setConfig(Settings *config) {
    this->config = config;
    if (config->scraper == "cache") {
        if (config->gameListVariants.contains("enable-manuals"))
            config->manuals = true;
        if (config->gameListVariants.contains("enable-fanart"))
            config->fanart = true;
    }
}

bool EmulationStation::loadOldGameList(const QString &gameListFileString) {
    // Load old game list entries so we can preserve metadata later when
    // assembling xml
    XmlReader gameListReader = XmlReader(config->inputFolder);
    if (gameListReader.setFile(gameListFileString)) {
        oldEntries = gameListReader.getEntries(extraGamelistTags(true));
        return true;
    }
    return false;
}

QStringList EmulationStation::extraGamelistTags(bool isFolder) {
    (void)isFolder; // don't care for ES
    GameEntry g;
    return g.extraTagNames(GameEntry::Format::RETROPIE, g);
}

void EmulationStation::skipExisting(QList<GameEntry> &gameEntries,
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
        if (ge.isFolder) {
            continue;
        }
        QFileInfo current(ge.path);
        for (auto qi = queue->begin(), end = queue->end(); qi != end; ++qi) {
            if (current.isFile()) {
                if (current.fileName() == (*qi).fileName()) {
                    queue->erase(qi);
                    qDebug() << "skipping game (file)" << current.fileName();
                    // We assume filename is unique, so break after getting
                    // first hit
                    break;
                }
            } else if (current.isDir()) {
                // Use current.canonicalFilePath here since it is already a
                // path. Otherwise it will use the parent folder
                if (current.canonicalFilePath() == (*qi).canonicalFilePath()) {
                    qDebug() << "skipping game (directory)"
                             << current.canonicalFilePath();
                    queue->erase(qi);
                    // We assume filename is unique, so break after getting
                    // first hit
                    break;
                }
            }
        }
    }
}

void EmulationStation::preserveVariants(const GameEntry &oldEntry,
                                        GameEntry &entry) {
    for (const auto &t : extraGamelistTags(entry.isFolder)) {
        if (entry.getEsExtra(t).isEmpty()) {
            entry.setEsExtra(t, oldEntry.getEsExtra(t));
        }
    }
}

void EmulationStation::preserveFromOld(GameEntry &entry) {
    for (const auto &oldEntry : oldEntries) {
        if (entry.path == oldEntry.path) {
            preserveVariants(oldEntry, entry);

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
                entry.fanartFile = oldEntry.fanartFile;
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
            entry.path = "./" + Config::lexicallyRelativePath(
                                    config->inputFolder, entry.path);
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
        for (auto &ext : exts) {
            if (sub.toLower().endsWith(ext.replace("*.", "."))) {
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

QString EmulationStation::openingElement(GameEntry &entry) {
    QString entryType = QString(entry.isFolder ? "folder" : "game");
    return QString(INDENT % "<" % entryType % ">");
}

QString EmulationStation::createXml(GameEntry &entry) {
    QStringList l;
    bool addEmptyElem = addEmptyElement() && !entry.isFolder;
    l.append(openingElement(entry));

    l.append(elem("path", entry.path, addEmptyElem));
    l.append(elem(GameEntry::getTag(GameEntry::Elem::TITLE), entry.title,
                  addEmptyElem));

    l += createEsVariantXml(entry);

    l.append(elem(GameEntry::getTag(GameEntry::Elem::RATING), entry.rating,
                  addEmptyElem));
    l.append(elem(GameEntry::getTag(GameEntry::Elem::DESCRIPTION),
                  entry.description, addEmptyElem));

    QString released = entry.releaseDate;
    QRegularExpressionMatch m = isoTimeRe().match(released);
    if (!m.hasMatch()) {
        released = released % "T000000";
    }
    l.append(elem(GameEntry::getTag(GameEntry::Elem::RELEASEDATE), released,
                  addEmptyElem));

    l.append(elem(GameEntry::getTag(GameEntry::Elem::DEVELOPER),
                  entry.developer, addEmptyElem));
    l.append(elem(GameEntry::getTag(GameEntry::Elem::PUBLISHER),
                  entry.publisher, addEmptyElem));
    l.append(elem(GameEntry::getTag(GameEntry::Elem::TAGS), entry.tags,
                  addEmptyElem));
    l.append(elem(GameEntry::getTag(GameEntry::Elem::PLAYERS), entry.players,
                  addEmptyElem));

    // write out non scraped elements
    const QString tagKidgame = GameEntry::getTag(GameEntry::Elem::AGES);
    for (const auto &t : extraGamelistTags(entry.isFolder)) {
        if (t != tagKidgame) {
            l.append(elem(t, entry.getEsExtra(t), false));
        }
    }
    QString kidGame = entry.getEsExtra(tagKidgame);
    if (kidGame.isEmpty() && entry.ages.toInt() >= 1 &&
        entry.ages.toInt() <= 10) {
        kidGame = "true";
    }

    l.append(elem(tagKidgame, kidGame, false));

    QString outerElemName = REGEX_OPENELEM.match(l[0]).captured(1);
    l.append(QString(INDENT % "</%1>").arg(outerElemName));
    l.removeAll("");

    return l.join("\n") % "\n";
}

QString EmulationStation::elem(const QString &elem, const QString &data,
                               bool addEmptyElem, bool isPath) {
    QString e;
    if (data.isEmpty()) {
        if (addEmptyElem) {
            e = QString(INDENT % INDENT % "<%1/>").arg(elem);
        }
    } else {
        QString fp = data;
        if (isPath) {
            if (config->relativePaths) {
                // fp is absolute, inputFolder is absolute
                // fp is always different from inputFolder
                // it is save to add "./" as it will always return sth relative
                fp = "./" +
                     Config::lexicallyRelativePath(config->inputFolder, fp);
            } else {
                // edge case when unattendSkip=true and a new game is added to
                // an existing gamelist with relative paths and relativePaths
                // has been changed from also true to false in the same run
                fp = Config::lexicallyNormalPath(fp);
            }
        }
        fp = StrTools::xmlEscape(fp);
        e = QString(INDENT % INDENT % "<%1>%2</%1>").arg(elem, fp);
    }
    return e;
}

QStringList EmulationStation::createEsVariantXml(const GameEntry &entry) {
    QStringList l;
    bool addEmptyElem = !entry.isFolder;
    l.append(elem(GameEntry::getTag(GameEntry::Elem::COVER), entry.coverFile,
                  addEmptyElem, true));
    l.append(elem(GameEntry::getTag(GameEntry::Elem::SCREENSHOT),
                  entry.screenshotFile, addEmptyElem, true));
    l.append(elem(GameEntry::getTag(GameEntry::Elem::MARQUEE),
                  entry.marqueeFile, addEmptyElem, true));
    l.append(elem(GameEntry::getTag(GameEntry::Elem::TEXTURE),
                  entry.textureFile, addEmptyElem, true));

    QString vidFile = entry.videoFile;
    if (!config->videos) {
        vidFile = "";
    }
    l.append(elem(GameEntry::getTag(GameEntry::Elem::VIDEO), vidFile,
                  addEmptyElem, true));

    if (!entry.manualSrc.isEmpty() && config->manuals) {
        l.append(elem(GameEntry::getTag(GameEntry::Elem::MANUAL),
                      entry.manualFile, false, true));
    }
    if (!entry.fanartSrc.isEmpty() && config->fanart) {
        l.append(elem(GameEntry::getTag(GameEntry::Elem::FANART),
                      entry.fanartFile, false, true));
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

QString EmulationStation::getGameListFolder() {
    return QString(QDir::homePath() % "/RetroPie/roms/" % config->platform);
}

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

QString EmulationStation::getFanartsFolder() {
    /* for ES variants */
    return config->mediaFolder % "/fanarts";
}
