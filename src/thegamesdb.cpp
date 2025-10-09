/***************************************************************************
 *            thegamesdb.cpp
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

#include "thegamesdb.h"

#include "gameentry.h"
#include "platform.h"
#include "strtools.h"

#include <QDebug>
#include <QJsonArray>

TheGamesDb::TheGamesDb(Settings *config, QSharedPointer<NetManager> manager)
    : AbstractScraper(config, manager, MatchType::MATCH_MANY) {
    loadMaps();

    baseUrl = "https://api.thegamesdb.net/v1";
    searchUrlPre = baseUrl + "/Games/ByGameName?apikey=";
    gfxUrl = "https://cdn.thegamesdb.net/images/original";

    fetchOrder.append(GameEntry::Elem::RELEASEDATE);
    fetchOrder.append(GameEntry::Elem::DESCRIPTION);
    fetchOrder.append(GameEntry::Elem::TAGS);
    fetchOrder.append(GameEntry::Elem::PLAYERS);
    fetchOrder.append(GameEntry::Elem::AGES);
    fetchOrder.append(GameEntry::Elem::DEVELOPER);
    fetchOrder.append(GameEntry::Elem::PUBLISHER);
    fetchOrder.append(GameEntry::Elem::COVER);
    fetchOrder.append(GameEntry::Elem::SCREENSHOT);
    fetchOrder.append(GameEntry::Elem::WHEEL);
    fetchOrder.append(GameEntry::Elem::MARQUEE);
    fetchOrder.append(GameEntry::Elem::FANART);
}

void TheGamesDb::getSearchResults(QList<GameEntry> &gameEntries,
                                  QString searchName, QString platform) {
    const QVector<int> configuredPlfIds = getPlatformId(config->platform);
    QStringList pIds;
    for (const auto &p : configuredPlfIds) {
        if (p > 0) {
            pIds.append(QString::number(p));
        }
    }

    netComm->request(
        searchUrlPre +
        StrTools::unMagic(
            "187;161;217;126;172;149;202;122;163;197;163;219;162;171;203;197;"
            "139;151;215;173;122;206;161;162;200;216;217;123;124;215;200;170;"
            "171;132;158;155;215;120;149;169;140;164;122;154;178;174;160;172;"
            "157;131;210;161;203;137;159;117;205;166;162;139;171;169;210;163") +
        "&name=" + searchName + "&filter[platform]=" + pIds.join(","));
    q.exec();
    data = netComm->getData();

    jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isEmpty()) {
        return;
    }

    reqRemaining = jsonDoc.object()["remaining_monthly_allowance"].toInt();
    if (reqRemaining <= 0)
        printf("\033[1;31mYou've reached TheGamesdDb's request limit for this "
               "month.\033[0m\n");

    if (jsonDoc.object()["status"].toString() != "Success") {
        return;
    }
    if (jsonDoc.object()["data"].toObject()["count"].toInt() < 1) {
        return;
    }

    QJsonArray jsonGames =
        jsonDoc.object()["data"].toObject()["games"].toArray();

    const QStringList fields = {"game_title", "players",    "release_date",
                                "developers", "publishers", "genres",
                                "overview",   "rating",     "platform"};

    while (!jsonGames.isEmpty()) {
        QJsonObject jsonGame = jsonGames.first().toObject();

        GameEntry game;

        game.id = QString::number(jsonGame["id"].toInt());
        game.url =
            QString("%1/Games/ByGameID?id=%2&apikey=%3&fields=%4")
                .arg(baseUrl)
                .arg(game.id)
                .arg(QString(StrTools::unMagic(
                    "187;161;217;126;172;149;202;122;163;197;163;219;162;"
                    "171;203;197;139;151;215;173;122;206;161;162;200;216;"
                    "217;123;124;215;200;170;171;132;158;155;215;120;149;"
                    "169;140;164;122;154;178;174;160;172;157;131;210;161;"
                    "203;137;159;117;205;166;162;139;171;169;210;163")))
                .arg(fields.join(","));

        game.title = jsonGame["game_title"].toString();
        // Remove anything at the end with a parentheses. 'thegamesdb' has a
        // habit of adding for instance '(1993)' to the name.
        game.title = game.title.left(game.title.indexOf("(")).simplified();

        if (jsonGame["release_date"] != QJsonValue::Undefined) {
            game.releaseDate = jsonGame["release_date"].toString();
        }
        int foundPlafId = jsonGame["platform"].toInt();
        game.platform = platformMap[QString::number(foundPlafId)].toString();
        qDebug() << "Trying to match platform" << game.platform;
        bool matchPlafId = configuredPlfIds.contains(foundPlafId);
        if (matchPlafId || platformMatch(game.platform, platform)) {
            if (matchPlafId) {
                qDebug() << "exact platform id match "
                         << QString::number(foundPlafId) << "as"
                         << game.platform;
            } else {
                qDebug() << "platform matched by alias" << game.platform;
            }
            gameEntries.append(game);
        }
        jsonGames.removeFirst();
    }
}

void TheGamesDb::getGameData(GameEntry &game) {
    qDebug() << "Per game url:" << game.url;
    netComm->request(game.url);
    q.exec();
    data = netComm->getData();
    jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isEmpty()) {
        printf("No returned json data, is 'thegamesdb' down?\n");
        reqRemaining = 0;
    }

    reqRemaining = jsonDoc.object()["remaining_monthly_allowance"].toInt();

    if (jsonDoc.object()["data"].toObject()["count"].toInt() < 1) {
        printf("No returned json game document, is 'thegamesdb' down?\n");
        reqRemaining = 0;
    }

    jsonObj = jsonDoc.object()["data"]
                  .toObject()["games"]
                  .toArray()
                  .first()
                  .toObject();
    populateGameEntry(game);
}

void TheGamesDb::getReleaseDate(GameEntry &game) {
    if (jsonObj["release_date"] != QJsonValue::Undefined)
        game.releaseDate = jsonObj["release_date"].toString();
}

void TheGamesDb::getDeveloper(GameEntry &game) {
    QJsonArray developers = jsonObj["developers"].toArray();
    if (developers.count() != 0)
        game.developer =
            developerMap[QString::number(developers.first().toInt())]
                .toString();
}

void TheGamesDb::getPublisher(GameEntry &game) {
    QJsonArray publishers = jsonObj["publishers"].toArray();
    if (publishers.count() != 0)
        game.publisher =
            publisherMap[QString::number(publishers.first().toInt())]
                .toString();
}

void TheGamesDb::getDescription(GameEntry &game) {
    game.description = jsonObj["overview"].toString();
}

void TheGamesDb::getPlayers(GameEntry &game) {
    int players = jsonObj["players"].toInt();
    if (players != 0)
        game.players = QString::number(players);
}

void TheGamesDb::getAges(GameEntry &game) {
    if (jsonObj["rating"] != QJsonValue::Undefined)
        game.ages = jsonObj["rating"].toString();
}

void TheGamesDb::getTags(GameEntry &game) {
    QJsonArray genres = jsonObj["genres"].toArray();
    if (genres.count() != 0) {
        while (!genres.isEmpty()) {
            QString id = QString::number(genres.first().toInt());
            QString g = genreMap[id].toString();
            qDebug() << "getTags():" << id << g;
            game.tags.append(g + ", ");
            genres.removeFirst();
        }
        game.tags.chop(2);
    }
}

void TheGamesDb::getCover(GameEntry &game) {
    QString req = gfxUrl + "/boxart/front/" + game.id + "-1";
    game.coverData = downloadMedia(req + ".jpg");
    if (game.coverData.isEmpty()) {
        game.coverData = downloadMedia(req + ".png");
    }
}

void TheGamesDb::getScreenshot(GameEntry &game) {
    QString req;
    QStringList exts = {".jpg", ".png"};
    // some platforms use screenshot/ rather than screenshots/
    QStringList pl = {"s/", "/"};
    int i = 0;
    do {
        int k = 0;
        do {
            req = gfxUrl + "/screenshot" + pl[k] + game.id + "-1" + exts[i];
            netComm->request(req);
            q.exec();
            k++;
        } while (k < 2 && netComm->getError() != QNetworkReply::NoError);
        i++;
    } while (i < 2 && netComm->getError() != QNetworkReply::NoError);

    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        game.screenshotData = netComm->getData();
        qDebug() << "tgdb: got screenshot from " << req;
    }
}

void TheGamesDb::getWheel(GameEntry &game) {
    QString req = gfxUrl + "/clearlogo/" + game.id + "-1";
    game.wheelData = downloadMedia(req + ".png");
    if (game.wheelData.isEmpty()) {
        // legacy, try without "-1"
        req.chop(2);
        game.wheelData = downloadMedia(req + ".png");
    }
}

void TheGamesDb::getMarquee(GameEntry &game) {
    QString req = gfxUrl + "/graphical/" + game.id + "-g";
    game.marqueeData = downloadMedia(req + ".jpg");
    if (game.marqueeData.isEmpty()) {
        game.marqueeData = downloadMedia(req + ".png");
    }
}

void TheGamesDb::getFanart(GameEntry &game) {
    // https://cdn.thegamesdb.net/images/original/fanart/189-1.jpg
    QString req = gfxUrl + "/fanart/" + game.id + "-1";
    game.fanartData = downloadMedia(req + ".jpg");
    if (game.fanartData.isEmpty()) {
        game.fanartData = downloadMedia(req + ".png");
    }
}

void TheGamesDb::loadMaps() {
    genreMap = readJson("tgdb_genres.json");
    developerMap = readJson("tgdb_developers.json");
    publisherMap = readJson("tgdb_publishers.json");
    platformMap = readJson("tgdb_platforms.json");
}

QVector<int> TheGamesDb::getPlatformId(const QString platform) {
    return Platform::get().getPlatformIdOnScraper(platform, config->scraper);
}