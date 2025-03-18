/***************************************************************************
 *            arcadedb.cpp
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

#include "arcadedb.h"

#include "strtools.h"

#include <QDebug>
#include <QJsonArray>
#include <QRegularExpression>

// match "Arcade Video game published NN years ago:"
static const QRegularExpression MATCH_YEARSAGO = QRegularExpression(
    "^.+\\s\\d+\\syears ago.*$", QRegularExpression::MultilineOption);
// match "Game (c) YYYY Corp."
static const QRegularExpression MATCH_COPYRIGHT = QRegularExpression(
    "^.+\\s\\([cC]\\).*\\d{2,4}.*$", QRegularExpression::MultilineOption);

ArcadeDB::ArcadeDB(Settings *config, QSharedPointer<NetManager> manager)
    : AbstractScraper(config, manager, MatchType::MATCH_ONE) {
    baseUrl = "http://adb.arcadeitalia.net";

    searchUrlPre =
        baseUrl +
        "/service_scraper.php?ajax=query_mame&lang=en&use_parent=1&game_name=";

    fetchOrder.append(PUBLISHER);
    fetchOrder.append(RELEASEDATE);
    fetchOrder.append(TAGS);
    fetchOrder.append(PLAYERS);
    fetchOrder.append(DESCRIPTION);
    fetchOrder.append(SCREENSHOT);
    fetchOrder.append(COVER);
    fetchOrder.append(WHEEL);
    fetchOrder.append(MARQUEE);
    fetchOrder.append(VIDEO);
}

void ArcadeDB::getSearchResults(QList<GameEntry> &gameEntries,
                                QString searchName, QString platform) {
    QString url = searchUrlPre + searchName;
    qDebug() << url;
    netComm->request(url);
    q.exec();
    data = netComm->getData();

    if (data.indexOf("{\"release\":1,\"result\":[]}") != -1) {
        return;
    }
    jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isEmpty()) {
        return;
    }
    jsonObj = jsonDoc.object().value("result").toArray().first().toObject();

    if (jsonObj.value("title") == QJsonValue::Undefined) {
        return;
    }

    GameEntry game;

    game.title = jsonObj.value("title").toString();
    game.platform = platform;
    game.releaseDate = jsonObj.value("year").toString();
    gameEntries.append(game);
}

void ArcadeDB::getGameData(GameEntry &game) { populateGameEntry(game); }

void ArcadeDB::getReleaseDate(GameEntry &game) {
    game.releaseDate = jsonObj.value("year").toString();
}

void ArcadeDB::getPlayers(GameEntry &game) {
    game.players = QString::number(jsonObj.value("players").toInt());
}

void ArcadeDB::getTags(GameEntry &game) {
    game.tags = jsonObj.value("genre").toString().replace(" / ", ", ");
}

void ArcadeDB::getPublisher(GameEntry &game) {
    game.publisher = jsonObj.value("manufacturer").toString();
}

void ArcadeDB::getDescription(GameEntry &game) {
    QString desc = jsonObj.value("history").toString();
    // these are usually the last paragraphs in a description
    QStringList ignoreList = {"- TECHNICAL", "- CONTRIBUTE"};
    for (auto const &kw : ignoreList) {
        if (desc.contains(kw)) {
            desc = desc.left(desc.indexOf(kw)).trimmed();
        }
    }
    desc = desc.replace(MATCH_YEARSAGO, "").trimmed();
    desc = desc.replace(MATCH_COPYRIGHT, "").trimmed();
    game.description = desc;
    qDebug() << desc;
}

void ArcadeDB::getCover(GameEntry &game) {
    // try flyer first, title (screen) as failsafe
    for (auto const &key :
         QStringList({"url_image_flyer", "url_image_title"})) {
        QString url = jsonObj.value(key).toString();
        if (url.isEmpty()) {
            continue;
        }

        netComm->request(url);
        q.exec();
        QImage image;
        if (netComm->getError() == QNetworkReply::NoError &&
            image.loadFromData(netComm->getData())) {
            game.coverData = netComm->getData();
            break;
        }
    }
}

void ArcadeDB::getScreenshot(GameEntry &game) {
    if (jsonObj.value("url_image_ingame").toString().isEmpty()) {
        return;
    }
    netComm->request(jsonObj.value("url_image_ingame").toString());
    q.exec();
    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        game.screenshotData = netComm->getData();
    }
}

void ArcadeDB::getWheel(GameEntry &game) {
    netComm->request(baseUrl + "/media/mame.current/decals/" +
                     jsonObj["game_name"].toString() + ".png");
    q.exec();
    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        game.wheelData = netComm->getData();
    }
}

void ArcadeDB::getMarquee(GameEntry &game) {
    if (jsonObj.value("url_image_marquee").toString().isEmpty()) {
        return;
    }
    netComm->request(jsonObj.value("url_image_marquee").toString());
    q.exec();
    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        game.marqueeData = netComm->getData();
    }
}

void ArcadeDB::getVideo(GameEntry &game) {
    for (auto const &key :
         QStringList({"url_video_shortplay_hd", "url_video_shortplay"})) {
        QString url = jsonObj.value(key).toString();
        if (url.isEmpty()) {
            continue;
        }
        netComm->request(url);
        q.exec();
        if (game.videoData = netComm->getData();
            netComm->getError() == QNetworkReply::NoError &&
            game.videoData.length() > 4096) {
            game.videoFormat = "mp4";
            break;
        } else {
            game.videoData.clear();
        }
    }
}

QList<QString> ArcadeDB::getSearchNames(const QFileInfo &info, QString &debug) {
    const QString baseName = info.completeBaseName();
    QList<QString> searchNames;
    debug.append("Base name: '" + baseName + "'\n");
    // don't use lookupSearchNames() to resolve ROM basename as provided MAME
    // Game ID is sufficient for this module, only do aliasMap lookup if user
    // decided to use it
    searchNames.append(lookupAliasMap(baseName, debug));
    return searchNames;
}
