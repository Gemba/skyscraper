/***************************************************************************
 *            igdb.cpp
 *
 *  Sun Aug 26 12:00:00 CEST 2018
 *  Copyright 2018 Lars Muldjord
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

#include "igdb.h"

#include "nametools.h"
#include "strtools.h"

#include <QJsonArray>

Igdb::Igdb(Settings *config, QSharedPointer<NetManager> manager)
    : AbstractScraper(config, manager, MatchType::MATCH_MANY) {
    QPair<QString, QString> clientIdHeader;
    clientIdHeader.first = "Client-ID";
    clientIdHeader.second = config->user;

    QPair<QString, QString> tokenHeader;
    tokenHeader.first = "Authorization";
    tokenHeader.second = "Bearer " + config->igdbToken;

    headers.append(clientIdHeader);
    headers.append(tokenHeader);

    connect(&limitTimer, &QTimer::timeout, &limiter, &QEventLoop::quit);
    limitTimer.setInterval(
        1100); // 1.1 second request limit set a bit above 1.0 as requested by
               // the good folks at IGDB. Don't change! It will break the module
               // stability.
    limitTimer.setSingleShot(false);
    limitTimer.start();

    baseUrl = "https://api.igdb.com/v4";

    searchUrlPre = "https://api.igdb.com/v4";

    fetchOrder.append(RELEASEDATE);
    fetchOrder.append(RATING);
    fetchOrder.append(PUBLISHER);
    fetchOrder.append(DEVELOPER);
    fetchOrder.append(DESCRIPTION);
    fetchOrder.append(PLAYERS);
    fetchOrder.append(TAGS);
    fetchOrder.append(AGES);
}

void Igdb::getSearchResults(QList<GameEntry> &gameEntries, QString searchName,
                            QString platform) {
    // Request list of games but don't allow re-releases ("game.version_parent =
    // null")
    limiter.exec();
    netComm->request(baseUrl + "/search/",
                     "fields "
                     "game.name,game.platforms.name,game.release_dates.date,"
                     "game.release_dates.platform; search \"" +
                         searchName +
                         "\"; where game != null & game.version_parent = null;",
                     headers);
    q.exec();
    data = netComm->getData();

    jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isEmpty()) {
        return;
    }

    if (jsonDoc.object()["message"].toString() == "Too Many Requests") {
        printf("\033[1;31mThe IGDB requests per second limit has been "
               "exceeded, can't continue!\033[0m\n");
        reqRemaining = 0;
        return;
    }

    QJsonArray jsonGames = jsonDoc.array();

    for (const auto &jsonGame : jsonGames) {
        GameEntry game;

        game.title = jsonGame.toObject()["game"].toObject()["name"].toString();
        game.id = QString::number(
            jsonGame.toObject()["game"].toObject()["id"].toInt());

        QJsonArray jsonPlatforms =
            jsonGame.toObject()["game"].toObject()["platforms"].toArray();
        for (const auto &jsonPlatform : jsonPlatforms) {
            int platformId = jsonPlatform.toObject()["id"].toInt();
            game.id.append(";" + QString::number(platformId));
            game.platform = jsonPlatform.toObject()["name"].toString();
            if (platformMatch(game.platform, platform)) {
                QJsonArray jsonReleaseDates = jsonGame.toObject()["game"]
                                                  .toObject()["release_dates"]
                                                  .toArray();
                for (const auto &releaseDate : jsonReleaseDates) {
                    if (releaseDate.toObject()["platform"].toInt() ==
                        platformId) {
                        game.releaseDate =
                            QDateTime::fromMSecsSinceEpoch(
                                (qint64)releaseDate.toObject()["date"].toInt() *
                                1000)
                                .date()
                                .toString(Qt::ISODate);
                    }
                }
                gameEntries.append(game);
            }
        }
    }
}

void Igdb::getGameData(GameEntry &game) {
    limiter.exec();
    netComm->request(
        baseUrl + "/games/",
        "fields "
        "age_ratings.rating,age_ratings.category,total_rating,cover.url,game_"
        "modes.slug,genres.name,screenshots.url,summary,release_dates.date,"
        "release_dates.region,release_dates.platform,involved_companies."
        "company.name,involved_companies.developer,involved_companies."
        "publisher; where id = " +
            game.id.split(";").first() + ";",
        headers);
    q.exec();
    data = netComm->getData();

    jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isEmpty()) {
        return;
    }

    jsonObj = jsonDoc.array().first().toObject();
    populateGameEntry(game);
}

void Igdb::getReleaseDate(GameEntry &game) {
    QJsonArray jsonDates = jsonObj["release_dates"].toArray();
    bool regionMatch = false;
    for (const auto &region : regionPrios) {
        for (const auto &jsonDate : jsonDates) {
            int regionEnum = jsonDate.toObject()["region"].toInt();
            QString curRegion = "";
            if (regionEnum == 1)
                curRegion = "eu";
            else if (regionEnum == 2)
                curRegion = "us";
            else if (regionEnum == 3)
                curRegion = "au";
            else if (regionEnum == 4)
                curRegion = "nz";
            else if (regionEnum == 5)
                curRegion = "jp";
            else if (regionEnum == 6)
                curRegion = "cn";
            else if (regionEnum == 7)
                curRegion = "asi";
            else if (regionEnum == 8)
                curRegion = "wor";
            if (QString::number(jsonDate.toObject()["platform"].toInt()) ==
                    game.id.split(";").last() &&
                region == curRegion) {
                game.releaseDate =
                    QDateTime::fromMSecsSinceEpoch(
                        (qint64)jsonDate.toObject()["date"].toInt() * 1000)
                        .toString("yyyy-MM-dd");
                regionMatch = true;
                break;
            }
        }
        if (regionMatch)
            break;
    }
}

void Igdb::getPlayers(GameEntry &game) {
    // This is a bit of a hack. The unique identifiers are as follows:
    // 1 = Single Player
    // 2 = Multiplayer
    // 3 = Cooperative
    // 4 = Split screen
    // 5 = MMO
    // So basically if != 1 it's at least 2 players. That's all we can gather
    // from this
    game.players = "1";
    QJsonArray jsonPlayers = jsonObj["game_modes"].toArray();
    for (const auto &jsonPlayer : jsonPlayers) {
        if (jsonPlayer.toObject()["id"].toInt() != 1) {
            game.players = "2";
            break;
        }
    }
}

void Igdb::getTags(GameEntry &game) {
    QJsonArray jsonGenres = jsonObj["genres"].toArray();
    for (const auto &jsonGenre : jsonGenres) {
        game.tags.append(jsonGenre.toObject()["name"].toString() + ", ");
    }
    game.tags.chop(2);
}

void Igdb::getAges(GameEntry &game) {
    int agesEnum =
        jsonObj["age_ratings"].toArray().first().toObject()["rating"].toInt();
    if (agesEnum == 1) {
        game.ages = "3";
    } else if (agesEnum == 2) {
        game.ages = "7";
    } else if (agesEnum == 3) {
        game.ages = "12";
    } else if (agesEnum == 4) {
        game.ages = "16";
    } else if (agesEnum == 5) {
        game.ages = "18";
    } else if (agesEnum == 6) {
        // Rating pending
    } else if (agesEnum == 7) {
        game.ages = "EC";
    } else if (agesEnum == 8) {
        game.ages = "E";
    } else if (agesEnum == 9) {
        game.ages = "E10";
    } else if (agesEnum == 10) {
        game.ages = "T";
    } else if (agesEnum == 11) {
        game.ages = "M";
    } else if (agesEnum == 12) {
        game.ages = "AO";
    }
}

void Igdb::getPublisher(GameEntry &game) {
    QJsonArray jsonCompanies = jsonObj["involved_companies"].toArray();
    for (const auto &jsonCompany : jsonCompanies) {
        if (jsonCompany.toObject()["publisher"].toBool() == true) {
            game.publisher =
                jsonCompany.toObject()["company"].toObject()["name"].toString();
            return;
        }
    }
}

void Igdb::getDeveloper(GameEntry &game) {
    QJsonArray jsonCompanies = jsonObj["involved_companies"].toArray();
    for (const auto &jsonCompany : jsonCompanies) {
        if (jsonCompany.toObject()["developer"].toBool() == true) {
            game.developer =
                jsonCompany.toObject()["company"].toObject()["name"].toString();
            return;
        }
    }
}

void Igdb::getDescription(GameEntry &game) {
    QJsonValue jsonValue = jsonObj["summary"];
    if (jsonValue != QJsonValue::Undefined) {
        game.description = StrTools::stripHtmlTags(jsonValue.toString());
    }
}

void Igdb::getRating(GameEntry &game) {
    QJsonValue jsonValue = jsonObj["total_rating"];
    if (jsonValue != QJsonValue::Undefined) {
        double rating = jsonValue.toDouble();
        if (rating != 0.0) {
            game.rating = QString::number(rating / 100.0);
        }
    }
}

QList<QString> Igdb::getSearchNames(const QFileInfo &info, QString &debug) {
    const QString baseName = info.completeBaseName();
    QString searchName = baseName;

    debug.append("Base name: '" + baseName + "'\n");

    searchName = lookupSearchName(info, baseName, debug);
    searchName = StrTools::stripBrackets(searchName);
    return QList<QString>{searchName};
}
