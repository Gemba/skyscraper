/***************************************************************************
 *            esgamelist.cpp
 *
 *  Mon Dec 17 08:00:00 CEST 2018
 *  Copyright 2018 Lars Muldjord & Martin Gerhardy
 *  Copyright 2025 Gemba @ GitHub
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

#include "esgamelist.h"

#include "config.h"
#include "gameentry.h"
#include "nametools.h"

ESGameList::ESGameList(Settings *config, QSharedPointer<NetManager> manager)
    : AbstractScraper(config, manager, MatchType::MATCH_ONE) {
    QDomDocument xmlDoc;
    QFile gameListFile(
        Config::concatPath(config->gameListFolder, config->gameListFilename));
    if (gameListFile.open(QIODevice::ReadOnly)) {
        xmlDoc.setContent(&gameListFile);
        gameListFile.close();
        games = xmlDoc.elementsByTagName("game");
    }
}

void ESGameList::getSearchResults(QList<GameEntry> &gameEntries,
                                  QString searchName, QString platform) {
    if (games.isEmpty()) {
        if (!gameListFile.exists()) {
            printf("\033[1;31mGamelist file not found '%s'. Now "
                   "quitting...\033[0m\n",
                   gameListFile.fileName().toStdString().c_str());
            exit(1);
        }
        printf("\033[1;33mGamelist file is empty '%s'. Continuing "
               "anyway.\033[0m\n",
               gameListFile.fileName().toStdString().c_str());
        return;
    }

    gameNode.clear();

    for (int i = 0; i < games.size(); ++i) {
        // Find <game> where last part of <path> matches file name
        QFileInfo info(games.item(i).firstChildElement("path").text());
        if (info.fileName() == searchName) {
            gameNode = games.item(i);
            GameEntry game;
            game.title = getElementText(GameEntry::Elem::TITLE);
            game.platform = platform;
            gameEntries.append(game);
            break;
        }
    }
}

QString ESGameList::getElementText(GameEntry::Elem key) {
    return gameNode.firstChildElement(GameEntry::getTag(key)).text();
}

void ESGameList::getGameData(GameEntry &game) {
    if (gameNode.isNull())
        return;

    game.releaseDate = getElementText(GameEntry::Elem::RELEASEDATE);
    game.publisher = getElementText(GameEntry::Elem::PUBLISHER);
    game.developer = getElementText(GameEntry::Elem::DEVELOPER);
    game.players = getElementText(GameEntry::Elem::PLAYERS);
    game.rating = getElementText(GameEntry::Elem::RATING);
    game.tags = getElementText(GameEntry::Elem::TAGS);
    game.description = getElementText(GameEntry::Elem::DESCRIPTION);

    if (config->cacheWheels) {
        // ES uses XML "marquee" but content is wheel (logo)
        game.wheelData =
            loadBinaryData(getElementText(GameEntry::Elem::MARQUEE));
    }
    if (config->cacheCovers) {
        game.coverData = loadBinaryData(getElementText(GameEntry::Elem::COVER));
    }
    if (config->cacheScreenshots) {
        game.screenshotData =
            loadBinaryData(getElementText(GameEntry::Elem::SCREENSHOT));
    }
    if (config->manuals) {
        game.manualData =
            loadBinaryData(getElementText(GameEntry::Elem::MANUAL));
    }
    if (config->fanart) {
        game.fanartData =
            loadBinaryData(getElementText(GameEntry::Elem::FANART));
    }
    if (config->backcovers) {
        game.backcoverData =
            loadBinaryData(getElementText(GameEntry::Elem::BACKCOVER));
    }
    if (config->videos) {
        loadVideoData(game, getElementText(GameEntry::Elem::VIDEO));
    }
}

QByteArray ESGameList::loadBinaryData(const QString fileName) {
    if (!fileName.isEmpty()) {
        QFile binFile(getAbsoluteFileName(fileName));
        if (binFile.open(QIODevice::ReadOnly)) {
            QByteArray data = binFile.readAll();
            binFile.close();
            return data;
        }
    }
    return QByteArray();
}

void ESGameList::loadVideoData(GameEntry &game, const QString fileName) {
    game.videoData = loadBinaryData(fileName);
    if (game.videoData.size() > 4096) {
        game.videoFormat = QFileInfo(getAbsoluteFileName(fileName)).suffix();
    }
}

QString ESGameList::getAbsoluteFileName(QString fileName) {
    /* paths in gamelist are relative to inputFolder for ES */
    fileName = Config::makeAbsolutePath(config->inputFolder, fileName);
    if (QFileInfo::exists(fileName)) {
        return fileName;
    }
    qDebug() << "file not found for" << config->inputFolder << fileName;
    return "";
}

QList<QString> ESGameList::getSearchNames(const QFileInfo &info,
                                          QString &debug) {
    QString fileName = info.fileName();
    debug.append("Filename: '" + fileName + "'\n");
    return QList<QString>({fileName});
}
