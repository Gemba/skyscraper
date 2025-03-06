/***************************************************************************
 *            igdb.cpp
 *
 *  Sun Aug 26 12:00:00 CEST 2018
 *  Copyright 2018 Lars Muldjord
 *  Copyright 2015 Gemba @ Github
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

#include "gameentry.h"
#include "nametools.h"
#include "strtools.h"

#include <QJsonArray>
#include <QRandomGenerator>

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
    /* 1.1 second request limit set a bit above 1.0 as requested by the good
     * folks at IGDB. Don't change! It will break the module stability. */
    limitTimer.setInterval(1100);
    limitTimer.setSingleShot(false);
    limitTimer.start();

    baseUrl = "https://api.igdb.com/v4";
    searchUrlPre = baseUrl;

    fetchOrder.append(RELEASEDATE);
    fetchOrder.append(RATING);
    fetchOrder.append(PUBLISHER);
    fetchOrder.append(DEVELOPER);
    fetchOrder.append(DESCRIPTION);
    fetchOrder.append(PLAYERS);
    fetchOrder.append(TAGS);
    fetchOrder.append(AGES);
    fetchOrder.append(SCREENSHOT);
    fetchOrder.append(COVER);
}

void Igdb::getSearchResults(QList<GameEntry> &gameEntries, QString searchName,
                            QString platform) {

    limiter.exec();
    const QStringList fields = {
        // clang-format off
        "game.name",
        "game.platforms.name",
        "game.release_dates.date",
        "game.release_dates.platform"
        // clang-format on
    };

    // Request list of games but don't allow re-releases ("game.version_parent =
    // null")
    QString clause =
        QString("search \"%1\"; where game != null").arg(searchName);
    if (searchName.startsWith("id=")) {
        // query by game id
        bool ok;
        int gameId = (searchName.split("=").at(1)).toInt(&ok);
        if (!ok) {
            gameId = -1;
        }
        clause = QString("where game = %1").arg(gameId);
    }

    const QString postData =
        QString("fields %1; %2 & game.version_parent = null;")
            .arg(fields.join(","))
            .arg(clause);
    qDebug() << baseUrl + "/search/";
    qDebug() << postData;
    netComm->request(baseUrl + "/search/", postData, headers);
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
        const QJsonObject gameObj = jsonGame.toObject()["game"].toObject();
        game.title = gameObj["name"].toString();
        game.id = QString::number(gameObj["id"].toInt());

        QJsonArray jsonPlatforms = gameObj["platforms"].toArray();
        QJsonArray jsonReleaseDates = gameObj["release_dates"].toArray();
        for (const auto &jsonPlatform : jsonPlatforms) {
            int platformId = jsonPlatform.toObject()["id"].toInt();
            game.id.append(";" + QString::number(platformId));
            game.platform = jsonPlatform.toObject()["name"].toString();
            if (platformMatch(game.platform, platform)) {
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
    const QStringList fields = {
        // clang-format off
        "age_ratings.organization",
        "age_ratings.rating_category",
        "cover.url",
        "game_modes.slug",
        "genres.name",
        "involved_companies.company.name",
        "involved_companies.developer",
        "involved_companies.publisher",
        "release_dates.date",
        "release_dates.platform",
        "release_dates.region",
        "screenshots.url",
        "summary",
        "total_rating"
        // clang-format on
    };
    const QString postData = QString("fields %1; where id = %2;")
                                 .arg(fields.join(","))
                                 .arg(game.id.split(";").first());
    netComm->request(baseUrl + "/games/", postData, headers);
    qDebug() << baseUrl + "/games/";
    qDebug() << postData;
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
    QStringList skyscraperRegions = {"eu", "us",  "au",  "nz", "jp",
                                     "cn", "asi", "wor", "kr", "br"};
    for (const auto &region : regionPrios) {
        for (const auto &jsonDate : jsonDates) {
            // cf. https://api.igdb.com/v4/release_date_regions
            int regionEnum = jsonDate.toObject()["region"].toInt();
            QString curRegion = "";
            if (regionEnum > 0 && regionEnum < skyscraperRegions.length()) {
                curRegion = skyscraperRegions.at(regionEnum);
            }
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
    // https://api.igdb.com/v4/age_ratings
    int agesEnum = jsonObj["age_ratings"]
                       .toArray()
                       .first()
                       .toObject()["rating_category"]
                       .toInt();
    switch (agesEnum) {
    case 1: // Three
        game.ages = "3";
        break;
    case 2:  // Seven
    case 28: // IND L
    case 34: // ACB G
        game.ages = "7";
        break;
    case 3:  // Twelve
    case 14: // CERO B
    case 20: // USK 12
    case 24: // GRAC Twelve
    case 30: // IND Twelve
        game.ages = "12";
        break;
    case 4:  // Sixteen
    case 21: // USK 16
    case 32: // IND Sixteen
    case 36: // ACB M
    case 37: // ACB MA15
        game.ages = "16";
        break;
    case 7: // EC
        game.ages = "EC";
        break;
    case 8:  // E
    case 13: // CERO A
    case 18: // USK 0
    case 23: // GRAC All
        game.ages = "E";
        break;
    case 9:  // E10
    case 29: // IND Ten
        game.ages = "E10";
        break;
    case 10: // T
        game.ages = "T";
        break;
    case 11: // M
        game.ages = "M";
        break;
    case 5:  // Eighteen
    case 12: // AO
    case 17: // CERO Z
    case 22: // USK 18
    case 33: // IND Eighteen
    case 38: // ACB R18
    case 39: // ACB RC
        game.ages = "AO";
        break;
    case 15: // CERO C
    case 25: // GRAC Fifteen
    case 35: // ACB PG
        game.ages = "15";
        break;
    case 16: // CERO D
        game.ages = "17";
        break;
    case 19: // USK 6
        game.ages = "6";
        break;
    case 26: // GRAC Eighteen
        // https://en.wikipedia.org/wiki/Game_Rating_and_Administration_Committee#Former_rating
        game.ages = "19";
        break;
    case 31: // IND Fourteen
        game.ages = "14";
        break;
    case 6:  // RP
    case 27: // GRAC Testing
    default:
        break;
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
        if (rating > 0.0) {
            rating = rating / 100.0;
            if (rating < 1.0) {
                rating += 0.005;
            }
            game.rating = QString::number(rating, 'g', 2);
        }
    }
}

void Igdb::getScreenshot(GameEntry &game) {
    QJsonArray mediaFiles = jsonObj["screenshots"].toArray();

    int chosen = 1;
    if (mediaFiles.count() < 1) {
        return;
    }
    const int offset = 2;
    if (mediaFiles.count() > offset) {
        // Claim: First 2 are almost always not ingame, so skip those if we have
        // 3 or more
#if QT_VERSION >= 0x050a00
        chosen =
            QRandomGenerator::system()->bounded(mediaFiles.count() - offset) +
            offset;
#else
        chosen = (qrand() % mediaFiles.count() - offset) + offset;
#endif
    }
    QString mediaUrl = mediaFiles.at(chosen).toObject()["url"].toString();

    QByteArray img = mediaFromJsonRef("screenshots", mediaUrl);
    if (!img.isEmpty()) {
        game.screenshotData = img;
    }
}

void Igdb::getCover(GameEntry &game) {
    QString mediaUrl = jsonObj["cover"].toObject()["url"].toString();
    QByteArray img = mediaFromJsonRef("cover", mediaUrl);
    if (!img.isEmpty()) {
        game.coverData = img;
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

QByteArray Igdb::mediaFromJsonRef(QString gameMedia, QString mediaUrl) {
    mediaUrl = mediaUrl.replace(QRegularExpression("^//"), "https://");
    mediaUrl = mediaUrl.replace("/t_thumb/", "/t_1080p/");
    qDebug() << gameMedia << mediaUrl;
    netComm->request(mediaUrl);
    q.exec();
    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        qDebug() << "success!";
        return netComm->getData();
    }
    return QByteArray();
}