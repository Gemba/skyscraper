/***************************************************************************
 *            worldofspectrum.cpp
 *
 *  Wed Jun 18 12:00:00 CEST 2017
 *  Copyright 2017 Lars Muldjord
 *
 *  04/2025: zxinfodk.cpp rewritten to use zxinfo.dk Web-API
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

#include "zxinfodk.h"

#include "gameentry.h"
#include "strtools.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPair>
#include <QRegularExpression>
#include <QStringBuilder>

static const QList<QPair<QString, QString>>
    headers({QPair("User-Agent", "Skyscraper / " VERSION)});
static const QString mediaUrl = "https://zxinfo.dk/media";

ZxInfoDk::ZxInfoDk(Settings *config, QSharedPointer<NetManager> manager)
    : AbstractScraper(config, manager, MatchType::MATCH_MANY) {
    baseUrl = "https://api.zxinfo.dk/v3";

    /* leave publisher commented here for capabilities-script for doc */
    // fetchOrder.append(PUBLISHER);
    fetchOrder.append(AGES);
    fetchOrder.append(DEVELOPER);
    fetchOrder.append(PLAYERS);
    fetchOrder.append(RATING);
    fetchOrder.append(RELEASEDATE);
    fetchOrder.append(TAGS);
    fetchOrder.append(COVER);
    fetchOrder.append(SCREENSHOT);
}

void ZxInfoDk::getSearchResults(QList<GameEntry> &gameEntries,
                                QString searchName, QString platform) {
    QString queryUrl = getQueryUrl(searchName);
    if (queryUrl.isEmpty())
        return;

    netComm->request(queryUrl, nullptr /* force GET */, headers);
    q.exec();
    data = netComm->getData();
    jsonDoc = QJsonDocument::fromJson(data);

    if (jsonDoc.isEmpty())
        return;

    if (queryUrl.contains("/search?")) {
        // at least one hit
        if (jsonDoc.object()["timed_out"].toBool(true)) {
            printf("\033[1;31mTimed out on server :(\033[0m\n");
            return;
        }
        QJsonArray jsonGamesHits =
            jsonDoc.object()["hits"].toObject()["hits"].toArray();

        for (const auto &hit : jsonGamesHits) {
            GameEntry game;
            QJsonObject jsonGameDetails = hit.toObject()["_source"].toObject();
            const QString id = hit.toObject()["_id"].toString();
            gameEntries.append(
                createMinimumGameEntry(id, jsonGameDetails, platform, false));
        }
    } else {
        // max. one hit
        QJsonObject jsonResult = jsonDoc.object();

        if (queryUrl.contains("/games/") && !jsonResult["found"].toBool()) {
            printf("\033[1;31mNot existing ID :/\033[0m\n");
            return;
        }

        GameEntry game;
        QJsonObject jsonGameDetails;
        QString id;
        if (queryUrl.contains("/games/")) {
            // id=...
            jsonGameDetails = jsonResult["_source"].toObject();
            id = jsonResult["_id"].toString();
        } else {
            // md5 / sha512
            jsonGameDetails = jsonResult;
            id = jsonResult["entry_id"].toString();
        }
        gameEntries.append(createMinimumGameEntry(
            id, jsonGameDetails, platform, queryUrl.contains("/filecheck/")));
    }
}

GameEntry ZxInfoDk::createMinimumGameEntry(const QString &id,
                                           const QJsonObject &jsonGameDetails,
                                           const QString &platform,
                                           bool filecheckQuery) {
    // get title, release year and publisher, set id, platform
    GameEntry game;
    game.id = id;
    game.platform = platform;

    game.title = jsonGameDetails["title"].toString();
    // prelimary: release year only
    game.releaseDate =
        QString::number(jsonGameDetails["originalYearOfRelease"].toInt());

    for (const auto &jsonPublisher : jsonGameDetails["publishers"].toArray()) {
        if (filecheckQuery ||
            jsonPublisher.toObject()["publisherSeq"].toInt() == 1) {
            game.publisher = jsonPublisher.toObject()["name"].toString();
            break;
        }
    }
    qDebug() << "zxinfo.dk got" << game.id << game.title << game.releaseDate
             << game.publisher;
    return game;
}

void ZxInfoDk::getGameData(GameEntry &game) {
    const QString gameUrl =
        QString("%1/games/%2?mode=compact").arg(baseUrl).arg(game.id, 7, '0');
    netComm->request(gameUrl, nullptr, headers);
    qDebug() << gameUrl;
    q.exec();
    data = netComm->getData();
    jsonDoc = QJsonDocument::fromJson(data);

    if (jsonDoc.isEmpty())
        return;

    jsonObj = jsonDoc.object()["_source"].toObject();
    populateGameEntry(game);
}

void ZxInfoDk::getCover(GameEntry &game) {
    for (const auto &jsonDlObj : jsonObj["additionalDownloads"].toArray()) {
        if (jsonDlObj.toObject()["type"].toString() == "Inlay - Front") {
            QString coverUrl =
                mediaUrl + jsonDlObj.toObject()["path"].toString();
            game.coverData = downloadMedia(coverUrl);
            if (!game.coverData.isEmpty()) {
                qDebug() << "got cover" << coverUrl;
                break;
            }
        }
    }
}

void ZxInfoDk::getScreenshot(GameEntry &game) {
    for (const auto &jsonDlObj : jsonObj["screens"].toArray()) {
        if (jsonDlObj.toObject()["type"].toString() == "Running screen") {
            QString scrUrl = mediaUrl + jsonDlObj.toObject()["url"].toString();
            game.screenshotData = downloadMedia(scrUrl);
            if (!game.screenshotData.isEmpty()) {
                qDebug() << "got screen" << scrUrl;
                break;
            }
        }
    }
}

void ZxInfoDk::getDeveloper(GameEntry &game) {
    QStringList devs;
    for (const auto &jsonAuthor : jsonObj["authors"].toArray()) {
        if (jsonAuthor.toObject()["type"].toString() == "Creator" &&
            jsonAuthor.toObject()["roles"].toArray().size() == 0) {
            devs.append(jsonAuthor.toObject()["name"].toString());
        }
        if (devs.length() > 3) {
            break;
        }
    }

    if (devs.length() > 3) {
        game.developer = devs[0] + ", et.al.";
    } else if (devs.length() > 2) {
        // abbrev. given names
        for (int k = 0; k < devs.size(); k++) {
            QStringList splitted = devs[k].split(" ");
            if (splitted.size() > 1) {
                devs[k] = splitted[0].left(1) % ". ";
                splitted.removeAt(0);
                devs[k] = devs[k] + splitted.join(" ");
            }
        }
    }
    if (devs.length() <= 3)
        game.developer = devs.join(", ");
}

void ZxInfoDk::getPlayers(GameEntry &game) {
    game.players = QString::number(jsonObj["numberOfPlayers"].toInt());
}

void ZxInfoDk::getAges(GameEntry &game) {
    game.ages = jsonObj["xrated"].toInt() != 0 ? "18" : "";
}

void ZxInfoDk::getTags(GameEntry &game) {
    game.tags = jsonObj["genreSubType"].toString();
    if (game.tags.isEmpty()) {
        game.tags = jsonObj["genreType"].toString();
    }
}

void ZxInfoDk::getRating(GameEntry &game) {
    double rating = jsonObj["score"].toObject()["score"].toDouble();
    game.rating = QString::number(rating / 10.0, 'g', 2);
}

void ZxInfoDk::getReleaseDate(GameEntry &game) {
    int yyyy = jsonObj["originalYearOfRelease"].toInt();
    int mm = jsonObj["originalMonthOfRelease"].toInt();
    int dd = jsonObj["originalDayOfRelease"].toInt();
    if (yyyy > 0) {
        game.releaseDate = QString::number(yyyy);
        if (mm > 0 && dd > 0) {
            QString mmdd = QString("-%1-%2")
                               .arg(QString::number(mm), 2, '0')
                               .arg(QString::number(dd), 2, '0');
            game.releaseDate.append(mmdd);
        }
    }
}

QString ZxInfoDk::getQueryUrl(QString searchName) {
    QString queryUrl;

    qDebug() << "input searchname" << searchName;
    QStringList params = {"mode=tiny", "titlesonly=true",
                          "contenttype=SOFTWARE", "genretype=GAMES"};

    int id = 0;
    if (searchName.toLower().startsWith("id=")) {
        searchName = searchName.toLower().replace("id=", "");
        id = searchName.toInt();
        if (id < 1 || id > 9999999) {
            printf("\033[1;31mProvided Id out of range.\033[0m\n");
            return queryUrl;
        }
    }
    if (id == 0) {
        bool isInt;
        id = searchName.toInt(&isInt);
        isInt =
            isInt && ((searchName.startsWith("0") && searchName.length() < 7) ||
                      searchName.length() == 7);
        if (!isInt) {
            // e.g. for "2048", force title search
            id = 0;
        }
    }
    if (id > 0) {
        queryUrl = QString("%1/games/%2?mode=compact")
                       .arg(baseUrl)
                       .arg(QString::number(id), 7, '0');
    } else if (QRegularExpression("^([a-f0-9]{32}|[a-f0-9]{128})$")
                   .match(searchName.toLower())
                   .hasMatch()) {
        queryUrl = QString("%1/filecheck/%2?%3")
                       .arg(baseUrl)
                       .arg(searchName.toLower())
                       .arg(params.join("&"));

    } else {
        searchName = searchName.replace("the+", "");
        params.append("query=" + searchName);
        queryUrl = QString("%1/search?%2").arg(baseUrl).arg(params.join("&"));
    }

    qDebug() << queryUrl;
    return queryUrl;
}
