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

#include "netcomm.h"
#include "platform.h"
#include "strtools.h"

#include <QJsonArray>
#include <QRandomGenerator>
#include <QStringBuilder>
#include <cstdio>

static inline QMap<QString, QString> mobyRegionMap() {
    return QMap<QString, QString>(
        {{"Australia", "au"},       {"Brazil", "br"},
         {"Bulgaria", "bg"},        {"Canada", "ca"},
         {"Chile", "cl"},           {"China", "cn"},
         {"Czech Republic", "cz"},  {"Denmark", "dk"},
         {"Finland", "fi"},         {"France", "fr"},
         {"Germany", "de"},         {"Greece", "gr"},
         {"Hungary", "hu"},         {"Israel", "il"},
         {"Italy", "it"},           {"Japan", "jp"},
         {"The Netherlands", "nl"}, {"New Zealand", "nz"},
         {"Norway", "no"},          {"Poland", "pl"},
         {"Portugal", "pt"},        {"Russia", "ru"},
         {"Slovakia", "sk"},        {"South Korea", "kr"},
         {"Spain", "sp"},           {"Sweden", "se"},
         {"Taiwan", "tw"},          {"Turkey", "tr"},
         {"United Kingdom", "uk"},  {"United States", "us"},
         {"Austria", "at"},         {"Belgium", "be"},
         {"Worldwide", "wor"}});
}

static inline QStringList stopWords = {"and", "&", "or", "to", "of", "by"};

MobyGames::MobyGames(Settings *config, QSharedPointer<NetManager> manager)
    : AbstractScraper(config, manager, MatchType::MATCH_MANY) {
    connect(&limitTimer, &QTimer::timeout, &limiter, &QEventLoop::quit);
    limitTimer.setInterval(5000); // 5 second request limit (Hobbyist API)
    limitTimer.setSingleShot(false);
    limitTimer.start();

    baseUrl = "https://api.mobygames.com";

    searchUrlPre = baseUrl + "/v2/games";

    fetchOrder.append(GameEntry::Elem::PUBLISHER);
    fetchOrder.append(GameEntry::Elem::DEVELOPER);
    fetchOrder.append(GameEntry::Elem::RELEASEDATE);
    fetchOrder.append(GameEntry::Elem::TAGS);
    // disabled: not in API v2
    // fetchOrder.append(GameEntry::Elem::P_LAYERS);
    // fetchOrder.append(GameEntry::Elem::A_GES);
    // fetchOrder.append(GameEntry::Elem::R_ATING);
    fetchOrder.append(GameEntry::Elem::DESCRIPTION);
    fetchOrder.append(GameEntry::Elem::COVER);
    fetchOrder.append(GameEntry::Elem::SCREENSHOT);

    mobyPlatformsMap = readJson("mobygames_platforms.json");
}

void MobyGames::getSearchResults(QList<GameEntry> &gameEntries,
                                 QString searchName, QString platform) {
    limiter.exec();
    // ignore '|' entries, only pick first
    int platformId = getPlatformId(config->platform)[0];

    printf("Waiting as advised by MobyGames api restrictions...");
    fflush(stdout);
    QString req =
        QString("%1?api_key=%2").arg(searchUrlPre).arg(config->password);
    bool isMobyGameId;
    int queryGameId = searchName.toInt(&isMobyGameId);
    if (isMobyGameId) {
        req = req % "&id=" % QString::number(queryGameId);
    } else {
        QString platformParam;
        if (platformId > 0)
            platformParam = "&platform=" % QString::number(platformId);
        req = req %
              QString("&title=%1%2&fuzzy=true&include=platforms,release_date")
                  .arg(searchName)
                  .arg(platformParam);
        queryGameId = 0;
    }

    if (!apiRequest(req)) {
        return;
    }

    QJsonArray jsonGames = jsonDoc.object()["games"].toArray();

    while (!jsonGames.isEmpty()) {
        GameEntry game;

        QJsonObject jsonGame = jsonGames.first().toObject();

        game.id = QString::number(jsonGame["game_id"].toInt());
        game.title = jsonGame["title"].toString();

        QJsonArray jsonPlatforms = jsonGame["platforms"].toArray();
        while (!jsonPlatforms.isEmpty()) {
            QJsonObject jsonPlatform = jsonPlatforms.first().toObject();
            gamePlatformId = jsonPlatform["platform_id"].toInt();
            bool matchPlafId = gamePlatformId == platformId;
            if (matchPlafId || platformMatch(game.platform, platform)) {
                game.url = searchUrlPre % "?id=" % game.id;
                game.releaseDate = jsonPlatform["release_date"].toString();
                game.platform = jsonPlatform["name"].toString();
                gameEntries.append(game);
                // assume only one platform match per game
                break;
            }
            jsonPlatforms.removeFirst();
        }
        jsonGames.removeFirst();
    }
}

QString MobyGames::removeStopwords(QString &searchName) {
    QStringList s;
    for (const auto &w : searchName.split(" ")) {
        if (!stopWords.contains(w)) {
            s.append(w);
        }
    }
    return s.join(" ");
}

void MobyGames::getGameData(GameEntry &game) {
    limiter.exec();
    printf("Waiting to get game data...");
    fflush(stdout);
    QStringList includes = {"covers",      "description", "developers",
                            "genres",      "publishers",  "release_date",
                            "screenshots", "title"};
    QString url = game.url % QString("&api_key=%1&include=%2")
                                 .arg(config->password)
                                 .arg(includes.join(","));
    if (!apiRequest(url)) {
        return;
    }

    jsonObj = jsonDoc.object()["games"].toArray()[0].toObject();
    populateGameEntry(game);
}

void MobyGames::getReleaseDate(GameEntry &game) {
    (void)game;
    // already done in initial game lookup
}

void MobyGames::getPlayers(GameEntry &game) {
    // TODO not via API - unsupported atm
    (void)game;
    /* v1 code
    QJsonArray jsonAttribs = jsonDoc.object()["attributes"].toArray();
    for (int a = 0; a < jsonAttribs.count(); ++a) {
        if (jsonAttribs.at(a)
                .toObject()["attribute_category_name"]
                .toString() == "Number of Players Supported") {
            game.players =
                jsonAttribs.at(a).toObject()["attribute_name"].toString();
        }
    }
    */
}

void MobyGames::getTags(GameEntry &game) {
    QJsonArray jsonGenres = jsonObj["genres"].toArray();
    for (auto gg : jsonGenres) {
        QJsonObject jg = gg.toObject();
        int genreCatId = jg["category_id"].toInt();
        qDebug() << "JSON genre id" << genreCatId;
        if (/*Basic Genres*/ 1 == genreCatId || /*Gameplay*/ 4 == genreCatId) {
            QString gs = jg["name"].toString();
            qDebug() << "Using" << gs << QString("(id %1)").arg(genreCatId);
            game.tags.append(gs % ", ");
        }
    }
    game.tags.chop(2);
}

void MobyGames::getAges(GameEntry &game) {
    // TODO not via API - unsupported atm
    (void)game;
    /*
    QJsonArray jsonAges = jsonDoc.object()["ratings"].toArray();
    QStringList ratingBodies = {"PEGI Rating",
                                "ELSPA Rating",
                                "ESRB Rating",
                                "USK Rating",
                                "OFLC (Australia) Rating",
                                "SELL Rating",
                                "BBFC Rating",
                                "OFLC (New Zealand) Rating",
                                "VRC Rating"};
    for (auto const &ja : jsonAges) {
        for (auto const &rb : ratingBodies) {
            if (auto jObj = ja.toObject();
                jObj["rating_system_name"].toString() == rb) {
                game.ages = jObj["rating_name"].toString();
                break;
            }
        }
        if (!game.ages.isEmpty()) {
            break;
        }
    }
    */
}

void MobyGames::getPublisher(GameEntry &game) {
    QJsonArray publishers = jsonObj["publishers"].toArray();
    for (int a = 0; a < publishers.count(); ++a) {
        QJsonArray publForPlatform = publishers.at(a)["platforms"].toArray();
        for (int b = 0; b < publForPlatform.count(); ++b) {
            if (publForPlatform.at(b).toString() == game.platform) {
                game.publisher = publishers.at(a).toObject()["name"].toString();
                return;
            }
        }
    }
}

void MobyGames::getDeveloper(GameEntry &game) {
    game.developer =
        jsonObj["developers"].toArray()[0].toObject()["name"].toString();
    qDebug() << "game.developer" << game.developer;
}

void MobyGames::getDescription(GameEntry &game) {
    game.description = jsonObj["description"].toString();

    // Remove all html tags within description
    game.description = StrTools::stripHtmlTags(game.description);
    // API Terms of Service
    game.description = game.description.trimmed();
    game.description.append("\n\nData by MobyGames.com");
}

void MobyGames::getRating(GameEntry &game) {
    // TODO not via API - unsupported atm
    (void)game;
    /* v1 code
    QJsonValue jsonValue = jsonObj["moby_score"];
    if (jsonValue != QJsonValue::Undefined) {
        double rating = jsonValue.toDouble();
        if (rating != 0.0) {
            game.rating = QString::number(rating / 10.0);
        }
    }
    */
}

void MobyGames::getCover(GameEntry &game) {
    printf("Retrieve front cover: ");

    QJsonArray covers = jsonObj["covers"].toArray();

    // pair: <cover index, front cover image index>
    QList<QPair<int, int>> matchIdx;

    for (int k = 0; k < covers.count(); k++) {
        QJsonArray plfs = covers.at(k).toObject()["platforms"].toArray();
        bool platMatched = false;
        while (!plfs.isEmpty()) {
            if (plfs.first().toObject()["id"] == gamePlatformId) {
                qDebug() << "platMatched!" << plfs.first().toObject()["name"];
                platMatched = true;
                break;
            }
            plfs.removeFirst();
        }
        if (!platMatched) {
            continue;
        } else {
            bool typeMatched = false;
            QJsonArray coverImgs = covers.at(k).toObject()["images"].toArray();
            int m = 0;
            while (!coverImgs.isEmpty()) {
                QJsonObject img = coverImgs.first().toObject();
                QJsonObject imgType = img["type"].toObject();
                if (imgType["id"].toInt() == 1) {
                    qDebug() << "typeMatched!" << img["image_url"];
                    typeMatched = true;
                    break;
                }
                coverImgs.removeFirst();
                m++;
            }
            // keep cover and image index
            if (typeMatched)
                matchIdx.append(QPair<int, int>(k, m));
        }
    }

    if (matchIdx.isEmpty()) {
        printf("No front covers at all for this platform.\n");
        return;
    }

    qDebug() << "Covers with matching platform with type 'Front Cover':"
             << matchIdx;

    QString coverUrl = "";
    QStringList foundRegions;
    QString regionMatch = "";
    qDebug() << regionPrios;
    for (const auto &region : regionPrios) {
        for (auto const &k : matchIdx) {
            QJsonArray jsonCountries =
                covers.at(k.first).toObject()["countries"].toArray();
            while (!jsonCountries.isEmpty()) {
                QString mobyCountry = getRegionShort(
                    jsonCountries.first()["name"].toString().trimmed());
                if (mobyCountry == region) {
                    regionMatch = region;
                    qDebug() << "region match" << region;
                    break;
                } else {
                    foundRegions.append(mobyCountry);
                }
                jsonCountries.removeFirst();
            }
            if (!regionMatch.isEmpty()) {
                QJsonArray coverImgs =
                    covers.at(k.first).toObject()["images"].toArray();
                coverUrl =
                    coverImgs.at(k.second).toObject()["image_url"].toString();
                break;
            }
        }
        if (!regionMatch.isEmpty()) {
            break;
        }
    }

    if (coverUrl.isEmpty()) {
        printf("No cover found for platform '%s' and these region prios\n"
               "  %s\n but for these regions\n  %s.\nYou may adjust your "
               "region prios to get a match.\n",
               game.platform.toStdString().c_str(),
               regionPrios.join(", ").toStdString().c_str(),
               foundRegions.join(", ").toStdString().c_str());
        return;
    }

    // For some reason the links are http but they
    // are always redirected to https
    coverUrl = coverUrl.replace("http://", "https://");
    qDebug() << coverUrl;
    game.coverData = downloadMedia(coverUrl);
    if (game.coverData.isEmpty()) {
        printf("Unexpected download or format error.\n");
        return;
    }
    QImage image;
    image.loadFromData(game.coverData);
    double aspect = image.height() / (double)image.width();
    if (aspect >= 0.8) {
        printf("OK. Region: '%s'\n", regionMatch.toStdString().c_str());
    } else {
        printf("Landscape mode detected. Cover discarded.\n");
        game.coverData.clear();
    }
}

void MobyGames::getScreenshot(GameEntry &game) {
    printf("Retrieve screenshot: ");
    fflush(stdout);
    QJsonArray jsonScreenshots = jsonObj["screenshots"].toArray();

    QString dlUrl;
    QString caption;
    int imgCount = 0;
    int pick = 0;
    for (int k = 0; k < jsonScreenshots.count(); k++) {
        QJsonObject screens = jsonScreenshots.at(k).toObject();
        // find screenshots for platform
        if (screens["platform_id"].toInt() == gamePlatformId) {
            QJsonArray jsonImgs = screens["images"].toArray();
            imgCount = jsonImgs.count();
            while (dlUrl.isEmpty()) {
                pick = QRandomGenerator::system()->bounded(imgCount);
                QJsonObject img = jsonImgs.at(pick).toObject();
                caption = img["caption"].toString().toLower();
                if (imgCount > 2 &&
                    (caption.contains("title") || caption.contains("intro"))) {
                    continue;
                }
                dlUrl = img["image_url"].toString();
            }
        }
        if (!dlUrl.isEmpty())
            break;
    }

    if (dlUrl.isEmpty()) {
        printf("No screenshots available.\n");
        return;
    }
    game.screenshotData = downloadMedia(dlUrl.replace("http://", "https://"));
    if (!game.screenshotData.isEmpty()) {
        printf("OK. Picked screenshot #%d of %d, caption '%s'.\n", pick,
               imgCount, caption.toStdString().c_str());
    } else {
        printf("No screenshot available.\n");
    }
}

QVector<int> MobyGames::getPlatformId(const QString platform) {
    return Platform::get().getPlatformIdOnScraper(platform, config->scraper);
}

QString MobyGames::getRegionShort(const QString &region) {
    if (mobyRegionMap().contains(region)) {
        // qDebug() << "resolved" << region << "to" << mobyRegionMap()[region];
        return mobyRegionMap()[region];
    }
    qWarning() << "Region not resolved for" << region;
    return "na";
}

bool MobyGames::apiRequest(const QString &url) {
    qDebug() << url;

    int retry = 0;
    while (retry < 5) {
        netComm->request(url);
        q.exec();
        data = netComm->getData();

        jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isEmpty()) {
            return false;
        }

        if (jsonDoc.object()["code"].toInt() == 429) {
            printf(".");
            fflush(stdout);
            qDebug() << "Got 429, round:" << retry;
            retry++;
            if (retry < 5)
                limiter.exec();
        } else {
            break;
        }
    }
    printf("\n");
    if (jsonDoc.object()["code"].toInt() == 429) {
        printf("\033[1;31mToo many requests signaled! Please wait a while and "
               "try again.\nNow quitting...\033[0m\n");
        reqRemaining = 0;
        return false;
    }
    return true;
}
