/***************************************************************************
 *            mobygames.cpp
 *
 *  Fri Mar 30 12:00:00 CEST 2018
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

#include "mobygames.h"

#include "platform.h"
#include "strtools.h"

#include <QJsonArray>

#if QT_VERSION >= 0x050a00
#include <QRandomGenerator>
#endif

MobyGames::MobyGames(Settings *config, QSharedPointer<NetManager> manager)
    : AbstractScraper(config, manager, MatchType::MATCH_MANY) {
    connect(&limitTimer, &QTimer::timeout, &limiter, &QEventLoop::quit);
    limitTimer.setInterval(10000); // 10 second request limit
    limitTimer.setSingleShot(false);
    limitTimer.start();

    baseUrl = "https://api.mobygames.com";

    searchUrlPre = baseUrl + "/v1/games";

    fetchOrder.append(PUBLISHER);
    fetchOrder.append(DEVELOPER);
    fetchOrder.append(RELEASEDATE);
    fetchOrder.append(TAGS);
    fetchOrder.append(PLAYERS);
    fetchOrder.append(DESCRIPTION);
    fetchOrder.append(AGES);
    fetchOrder.append(RATING);
    fetchOrder.append(COVER);
    fetchOrder.append(SCREENSHOT);
}

void MobyGames::getSearchResults(QList<GameEntry> &gameEntries,
                                 QString searchName, QString platform) {
    int platformId = getPlatformId(config->platform);

    printf("Waiting as advised by MobyGames api restrictions...\n");
    limiter.exec();
    bool ok;
    int queryGameId = searchName.toInt(&ok);
    QString req;
    if (ok) {
        req = QString(searchUrlPre + "?api_key=" +
                      StrTools::unMagic(
                          "175;229;170;189;188;202;211;117;164;165;185;209;164;"
                          "234;180;155;199;209;224;231;193;190;173;175") +
                      "&id=" + QString::number(queryGameId));
    } else {
        req = QString(searchUrlPre + "?api_key=" +
                      StrTools::unMagic(
                          "175;229;170;189;188;202;211;117;164;165;185;209;164;"
                          "234;180;155;199;209;224;231;193;190;173;175") +
                      "&title=" + searchName +
                      (platformId == -1
                           ? ""
                           : "&platform=" + QString::number(platformId)));
        queryGameId = 0;
    }
    qDebug() << "Request: " << req;
    netComm->request(req);
    q.exec();
    data = netComm->getData();

    jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isEmpty()) {
        return;
    }

    if (jsonDoc.object()["code"].toInt() == 429) {
        printf(
            "\033[1;31mToo many requests! This is probably because some other "
            "Skyscraper user is currently using the 'mobygames' module. Please "
            "wait a while and try again.\n\nNow quitting...\033[0m\n");
        reqRemaining = 0;
    }

    QJsonArray jsonGames = jsonDoc.object()["games"].toArray();

    while (!jsonGames.isEmpty()) {
        GameEntry game;

        QJsonObject jsonGame = jsonGames.first().toObject();

        game.id = QString::number(jsonGame["game_id"].toInt());
        game.title = jsonGame["title"].toString();
        game.miscData = QJsonDocument(jsonGame).toJson(QJsonDocument::Compact);

        QJsonArray jsonPlatforms = jsonGame["platforms"].toArray();
        while (!jsonPlatforms.isEmpty()) {
            QJsonObject jsonPlatform = jsonPlatforms.first().toObject();
            int gamePlafId = jsonPlatform["platform_id"].toInt();
            game.url = searchUrlPre + "/" + game.id + "/platforms/" +
                       QString::number(gamePlafId) + "?api_key=" +
                       StrTools::unMagic(
                           "175;229;170;189;188;202;211;117;164;165;185;209;"
                           "164;234;180;155;199;209;224;231;193;190;173;175");
            game.releaseDate = jsonPlatform["first_release_date"].toString();
            game.platform = jsonPlatform["platform_name"].toString();
            bool matchPlafId = gamePlafId == platformId;
            if (platformMatch(game.platform, platform) || matchPlafId) {
                gameEntries.append(game);
            }
            jsonPlatforms.removeFirst();
        }
        jsonGames.removeFirst();
    }
}

void MobyGames::getGameData(GameEntry &game) {
    printf("Waiting to get game data...\n");
    limiter.exec();
    netComm->request(game.url);
    q.exec();
    data = netComm->getData();

    jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isEmpty()) {
        return;
    }

    jsonObj = QJsonDocument::fromJson(game.miscData).object();
    populateGameEntry(game);
}

void MobyGames::getReleaseDate(GameEntry &game) {
    game.releaseDate = jsonDoc.object()["first_release_date"].toString();
}

void MobyGames::getPlayers(GameEntry &game) {
    QJsonArray jsonAttribs = jsonDoc.object()["attributes"].toArray();
    for (int a = 0; a < jsonAttribs.count(); ++a) {
        if (jsonAttribs.at(a)
                .toObject()["attribute_category_name"]
                .toString() == "Number of Players Supported") {
            game.players =
                jsonAttribs.at(a).toObject()["attribute_name"].toString();
        }
    }
}

void MobyGames::getTags(GameEntry &game) {
    QJsonArray jsonGenres = jsonObj["genres"].toArray();
    for (auto gg : jsonGenres) {
        QJsonObject jg = gg.toObject();
        int genreCatId = jg["genre_category_id"].toInt();
        qDebug() << "Got genre cat id" << genreCatId << "\n";
        if (/*Basic Genres*/ 1 == genreCatId || /*Gameplay*/ 4 == genreCatId) {
            QString gs = jg["genre_name"].toString();
            game.tags.append(gs + ", ");
            qDebug() << "Using" << gs;
        }
    }
    game.tags.chop(2);
}

void MobyGames::getAges(GameEntry &game) {
    QJsonArray jsonAges = jsonDoc.object()["ratings"].toArray();
    for (int a = 0; a < jsonAges.count(); ++a) {
        if (jsonAges.at(a).toObject()["rating_system_name"].toString() ==
            "PEGI Rating") {
            game.ages = jsonAges.at(a).toObject()["rating_name"].toString();
            break;
        }
    }
    for (int a = 0; a < jsonAges.count(); ++a) {
        if (jsonAges.at(a).toObject()["rating_system_name"].toString() ==
            "ELSPA Rating") {
            game.ages = jsonAges.at(a).toObject()["rating_name"].toString();
            break;
        }
    }
    for (int a = 0; a < jsonAges.count(); ++a) {
        if (jsonAges.at(a).toObject()["rating_system_name"].toString() ==
            "ESRB Rating") {
            game.ages = jsonAges.at(a).toObject()["rating_name"].toString();
            break;
        }
    }
    for (int a = 0; a < jsonAges.count(); ++a) {
        if (jsonAges.at(a).toObject()["rating_system_name"].toString() ==
            "USK Rating") {
            game.ages = jsonAges.at(a).toObject()["rating_name"].toString();
            break;
        }
    }
    for (int a = 0; a < jsonAges.count(); ++a) {
        if (jsonAges.at(a).toObject()["rating_system_name"].toString() ==
            "OFLC (Australia) Rating") {
            game.ages = jsonAges.at(a).toObject()["rating_name"].toString();
            break;
        }
    }
    for (int a = 0; a < jsonAges.count(); ++a) {
        if (jsonAges.at(a).toObject()["rating_system_name"].toString() ==
            "SELL Rating") {
            game.ages = jsonAges.at(a).toObject()["rating_name"].toString();
            break;
        }
    }
    for (int a = 0; a < jsonAges.count(); ++a) {
        if (jsonAges.at(a).toObject()["rating_system_name"].toString() ==
            "BBFC Rating") {
            game.ages = jsonAges.at(a).toObject()["rating_name"].toString();
            break;
        }
    }
    for (int a = 0; a < jsonAges.count(); ++a) {
        if (jsonAges.at(a).toObject()["rating_system_name"].toString() ==
            "OFLC (New Zealand) Rating") {
            game.ages = jsonAges.at(a).toObject()["rating_name"].toString();
            break;
        }
    }
    for (int a = 0; a < jsonAges.count(); ++a) {
        if (jsonAges.at(a).toObject()["rating_system_name"].toString() ==
            "VRC Rating") {
            game.ages = jsonAges.at(a).toObject()["rating_name"].toString();
            break;
        }
    }
}

void MobyGames::getPublisher(GameEntry &game) {
    QJsonArray jsonReleases = jsonDoc.object()["releases"].toArray();
    for (int a = 0; a < jsonReleases.count(); ++a) {
        QJsonArray jsonCompanies =
            jsonReleases.at(a).toObject()["companies"].toArray();
        for (int b = 0; b < jsonCompanies.count(); ++b) {
            if (jsonCompanies.at(b).toObject()["role"].toString() ==
                "Published by") {
                game.publisher =
                    jsonCompanies.at(b).toObject()["company_name"].toString();
                return;
            }
        }
    }
}

void MobyGames::getDeveloper(GameEntry &game) {
    QJsonArray jsonReleases = jsonDoc.object()["releases"].toArray();
    for (int a = 0; a < jsonReleases.count(); ++a) {
        QJsonArray jsonCompanies =
            jsonReleases.at(a).toObject()["companies"].toArray();
        for (int b = 0; b < jsonCompanies.count(); ++b) {
            if (jsonCompanies.at(b).toObject()["role"].toString() ==
                "Developed by") {
                game.developer =
                    jsonCompanies.at(b).toObject()["company_name"].toString();
                return;
            }
        }
    }
}

void MobyGames::getDescription(GameEntry &game) {
    game.description = jsonObj["description"].toString();

    // Remove all html tags within description
    game.description = StrTools::stripHtmlTags(game.description);
}

void MobyGames::getRating(GameEntry &game) {
    QJsonValue jsonValue = jsonObj["moby_score"];
    if (jsonValue != QJsonValue::Undefined) {
        double rating = jsonValue.toDouble();
        if (rating != 0.0) {
            game.rating = QString::number(rating / 10.0);
        }
    }
}

void MobyGames::getCover(GameEntry &game) {
    printf("Waiting to get cover data...\n");
    limiter.exec();
    QString req = QString(
        game.url.left(game.url.indexOf("?api_key=")) + "/covers" +
        game.url.mid(game.url.indexOf("?api_key="),
                     game.url.length() - game.url.indexOf("?api_key=")));
    qDebug() << "Covers " << req;
    netComm->request(req);
    q.exec();
    data = netComm->getData();

    jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isEmpty()) {
        return;
    }

    QString coverUrl = "";
    bool foundFrontCover = false;

    for (const auto &region : regionPrios) {
        QJsonArray jsonCoverGroups = jsonDoc.object()["cover_groups"].toArray();
        while (!jsonCoverGroups.isEmpty()) {
            bool foundRegion = false;
            QJsonArray jsonCountries =
                jsonCoverGroups.first().toObject()["countries"].toArray();
            while (!jsonCountries.isEmpty()) {
                if (getRegionShort(
                        jsonCountries.first().toString().simplified()) ==
                    region) {
                    foundRegion = true;
                    break;
                }
                jsonCountries.removeFirst();
            }
            if (!foundRegion) {
                jsonCoverGroups.removeFirst();
                continue;
            }
            QJsonArray jsonCovers =
                jsonCoverGroups.first().toObject()["covers"].toArray();
            while (!jsonCovers.isEmpty()) {
                QJsonObject jsonCover = jsonCovers.first().toObject();
                if (jsonCover["scan_of"]
                        .toString()
                        .toLower()
                        .simplified()
                        .contains("front cover")) {
                    coverUrl = jsonCover["image"].toString();
                    foundFrontCover = true;
                    break;
                }
                jsonCovers.removeFirst();
            }
            if (foundFrontCover) {
                break;
            }
            jsonCoverGroups.removeFirst();
        }
        if (foundFrontCover) {
            break;
        }
    }

    // For some reason the links are http but they
    // are always redirected to https
    coverUrl.replace("http://", "https://");

    if (!coverUrl.isEmpty()) {
        netComm->request(coverUrl);
        q.exec();
        QImage image;
        if (netComm->getError() == QNetworkReply::NoError &&
            image.loadFromData(netComm->getData())) {
            game.coverData = netComm->getData();
        }
    }
}

void MobyGames::getScreenshot(GameEntry &game) {
    printf("Waiting to get screenshot data...\n");
    limiter.exec();
    netComm->request(
        game.url.left(game.url.indexOf("?api_key=")) + "/screenshots" +
        game.url.mid(game.url.indexOf("?api_key="),
                     game.url.length() - game.url.indexOf("?api_key=")));
    q.exec();
    data = netComm->getData();

    jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isEmpty()) {
        return;
    }

    QJsonArray jsonScreenshots = jsonDoc.object()["screenshots"].toArray();

    if (jsonScreenshots.count() < 1) {
        return;
    }
    int chosen = 1;
    if (jsonScreenshots.count() > 2) {
        // First 2 are almost always not ingame, so skip those if we have 3 or
        // more
#if QT_VERSION >= 0x050a00
        chosen =
            (QRandomGenerator::system()->bounded(jsonScreenshots.count() - 2)) +
            2;
#else
        chosen = (qrand() % jsonScreenshots.count() - 2) + 2;
#endif
    }
    netComm->request(
        jsonScreenshots.at(chosen).toObject()["image"].toString().replace(
            "http://", "https://"));
    q.exec();
    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        game.screenshotData = netComm->getData();
    }
}

int MobyGames::getPlatformId(const QString platform) {
    return Platform::get().getPlatformIdOnScraper(platform, config->scraper);
}

QString MobyGames::getRegionShort(const QString &region) {
    if (regionMap.contains(region)) {
        qDebug() << "Got region" << regionMap[region];
        return regionMap[region];
    }
    return "na";
}

void MobyGames::setupRegionMap() {
    regionMap = QMap<QString, QString>(
        {{"Australia", "au"},      {"Brazil", "br"},
         {"Bulgaria", "bg"},       {"Canada", "ca"},
         {"Chile", "cl"},          {"China", "cn"},
         {"Czech Republic", "cz"}, {"Denmark", "dk"},
         {"Finland", "fi"},        {"France", "fr"},
         {"Germany", "de"},        {"Greece", "gr"},
         {"Hungary", "hu"},        {"Israel", "il"},
         {"Italy", "it"},          {"Japan", "jp"},
         {"Netherlands", "nl"},    {"New Zealand", "nz"},
         {"Norway", "no"},         {"Poland", "pl"},
         {"Portugal", "pt"},       {"Russia", "ru"},
         {"Slovakia", "sk"},       {"South Korea", "kr"},
         {"Spain", "sp"},          {"Sweden", "se"},
         {"Taiwan", "tw"},         {"Turkey", "tr"},
         {"United Kingdom", "uk"}, {"United States", "us"},
         {"Worldwide", "wor"}});
}
