/***************************************************************************
 *            openretro.cpp
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

#include "openretro.h"

#include "nametools.h"
#include "strtools.h"

#include <QDebug>
#include <QRegularExpression>

OpenRetro::OpenRetro(Settings *config, QSharedPointer<NetManager> manager)
    : AbstractScraper(config, manager, MatchType::MATCH_MANY) {
    baseUrl = "https://openretro.org";

    searchUrlPre = baseUrl;
    searchUrlPost = "&unpublished=1";

    searchResultPre = "<div style='margin-bottom: 4px;'>";
    urlPre.append("<a href=\"/");
    urlPost = "\" target='_parent'>";
    titlePre.append("</div>");
    titlePost = " <span";
    platformPre.append("#aaaaaa'>");
    platformPost = "</span>";
    marqueePre.append(">banner_sha1</td>");
    marqueePre.append("<div><a href=\"");
    marqueePost = "\">";
    // Check getDescription function for special case __long_description scrape
    // For description
    descriptionPre.append(">description</td>");
    descriptionPre.append("<td style='color: black;'><div>");
    // For __long_description
    descriptionPre.append("__long_description</td>");
    descriptionPre.append("<td style='color: black;'><div>");
    descriptionPost = "</div></td>";
    developerPre.append("black;'>developer</td>");
    developerPre.append("<td style='color: black;'><div>");
    developerPost = "</div></td>";
    coverPre.append(">front_sha1</td>");
    coverPre.append("<div><a href=\"");
    coverPost = "\">";
    playersPre.append(">players</td>");
    playersPre.append("<td style='color: black;'><div>");
    playersPost = "</div></td>";
    publisherPre.append(">publisher</td>");
    publisherPre.append("<td style='color: black;'><div>");
    publisherPost = "</div></td>";
    screenshotCounter = "<td style='width: 180px; color: black;'>screen";
    screenshotPre.append("<td style='width: 180px; color: black;'>screen");
    screenshotPre.append("<td style='color: black;'><div><a href=\"");
    screenshotPost = "\">";
    releaseDatePre.append(">year</td>");
    releaseDatePre.append("<td style='color: black;'><div>");
    releaseDatePost = "</div></td>";
    tagsPre.append(">tags</td>");
    ratingPre.append("<div class='game-review'>");
    ratingPre.append("<span class='score'>");
    ratingPre.append("<div class='score'>Score: ");
    ratingPost = "</";

    fetchOrder.append(MARQUEE);
    fetchOrder.append(DESCRIPTION);
    fetchOrder.append(DEVELOPER);
    fetchOrder.append(COVER);
    fetchOrder.append(PLAYERS);
    fetchOrder.append(PUBLISHER);
    fetchOrder.append(SCREENSHOT);
    fetchOrder.append(TAGS);
    fetchOrder.append(RELEASEDATE);
    fetchOrder.append(RATING);
}

void OpenRetro::getSearchResults(QList<GameEntry> &gameEntries,
                                 QString searchName, QString platform) {
    bool hasWhdlUuid = searchName.left(6) == "/game/";
    QString lookupReq = QString(searchUrlPre);
    if (hasWhdlUuid) {
        lookupReq = lookupReq + searchName;
    } else {
        lookupReq = lookupReq + "/browse?q=" + searchName + searchUrlPost;
    }
    netComm->request(lookupReq);
    q.exec();

    GameEntry game;

    while (!netComm->getRedirUrl().isEmpty()) {
        game.url = netComm->getRedirUrl();
        netComm->request(game.url);
        q.exec();
    }
    data = netComm->getData();

    if (data.isEmpty())
        return;

    if (data.contains("Error: 500 Internal Server Error") ||
        data.contains("Error: 404 Not Found"))
        return;

    if (hasWhdlUuid) {
        QByteArray tempData = data;
        nomNom("<td style='width: 180px; color: black;'>game_name</td>");
        nomNom("<td style='color: black;'><div>");
        // Remove AGA, we already add this automatically in
        // StrTools::addSqrBrackets
        game.title = data.left(data.indexOf("</div></td>"))
                         .replace("[AGA]", "")
                         .replace("[CD32]", "")
                         .replace("[CDTV]", "")
                         .simplified();
        data = tempData;
        game.platform = platform;
        // Check if title is empty. Some games exist but have no data, not even
        // a name. We don't want those results
        if (!game.title.isEmpty())
            gameEntries.append(game);
    } else {
        while (data.indexOf(searchResultPre.toUtf8()) != -1) {
            nomNom(searchResultPre);

            // Digest until url
            for (const auto &nom : urlPre) {
                nomNom(nom);
            }
            game.url = baseUrl + "/" +
                       data.left(data.indexOf(urlPost.toUtf8())) + "/edit";

            // Digest until title
            for (const auto &nom : titlePre) {
                nomNom(nom);
            }
            // Remove AGA, we already add this automatically in
            // StrTools::addSqrBrackets
            game.title = data.left(data.indexOf(titlePost.toUtf8()))
                             .replace("[AGA]", "")
                             .simplified();

            // Digest until platform
            for (const auto &nom : platformPre) {
                nomNom(nom);
            }
            game.platform = data.left(data.indexOf(platformPost.toUtf8()))
                                .replace("&nbsp;", " ");

            if (platformMatch(game.platform, platform)) {
                gameEntries.append(game);
            }
        }
    }
}

void OpenRetro::getGameData(GameEntry &game) {
    if (!game.url.isEmpty()) {
        netComm->request(game.url);
        q.exec();
        data = netComm->getData();
    }

    // Remove all the variants so we don't choose between their screenshots
    data = data.left(data.indexOf("</table></div><div id='"));
    populateGameEntry(game);
}

void OpenRetro::getDescription(GameEntry &game) {
    if (descriptionPre.isEmpty()) {
        return;
    }
    QByteArray tempData = data;

    if (data.indexOf(descriptionPre.at(0).toUtf8()) != -1) {
        // If description
        nomNom(descriptionPre.at(0));
        nomNom(descriptionPre.at(1));
    } else if (data.indexOf(descriptionPre.at(2).toUtf8()) != -1) {
        // If __long_description
        nomNom(descriptionPre.at(2));
        nomNom(descriptionPre.at(3));
    } else {
        return;
    }

    game.description = data.left(data.indexOf(descriptionPost.toUtf8()))
                           .replace("&lt;", "<")
                           .replace("&gt;", ">");
    // Revert data back to pre-description
    data = tempData;

    // Remove all html tags within description
    game.description = StrTools::stripHtmlTags(game.description);
}

void OpenRetro::getTags(GameEntry &game) {
    for (const auto &nom : tagsPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : tagsPre) {
        nomNom(nom);
    }
    QString tags = "";
    QString tagBegin = "<a href=\"/browse/";
    while (data.indexOf(tagBegin.toUtf8()) != -1) {
        nomNom(tagBegin);
        nomNom("\">");
        tags.append(data.left(data.indexOf("</a>")) + ", ");
    }
    if (!tags.isEmpty()) {
        tags.chop(2); // Remove last ", "
    }

    game.tags = tags;
}

void OpenRetro::getRating(GameEntry &game) {
    game.url.chop(5); // remove trailing '/edit'
    netComm->request(game.url);
    q.exec();
    data = netComm->getData();

    bool ratingDecimal = true;

    if (checkNom(ratingPre.at(0))) {
        // "0 ... 100%" or "numerator/denominator" (from mags or ext. websites)
        nomNom(ratingPre.at(0));
        nomNom(ratingPre.at(1));
        ratingDecimal = false;
    } else if (checkNom(ratingPre.at(2))) {
        // 1.0 ... 10.0 (from Openretro)
        nomNom(ratingPre.at(2));
    } else {
        game.rating = "";
        return;
    }

    bool toDoubleOk = false;
    game.rating = data.left(data.indexOf(ratingPost.toUtf8()));
    qDebug() << "game.rating" << game.rating;
    qDebug() << "ratingDecimal" << ratingDecimal;

    if (!ratingDecimal) {
        if (game.rating.endsWith("%")) {
            game.rating.chop(1);
        } else if (game.rating.contains("/")) {
            QStringList parts = game.rating.split("/");
            double num = parts.value(0).toDouble(&toDoubleOk);
            if (toDoubleOk) {
                double den = parts.value(1).toDouble(&toDoubleOk);
                if (toDoubleOk && den > 0.0) {
                    game.rating = QString::number(num / den, 'g', 2);
                    return;
                }
            }
            game.rating = "";
        } else {
            game.rating = "";
        }
    }

    double rating = game.rating.toDouble(&toDoubleOk);
    if (toDoubleOk) {
        game.rating = QString::number(rating / (ratingDecimal ? 1.0 : 100.0));
    } else {
        game.rating = "";
    }
}

void OpenRetro::getCover(GameEntry &game) {
    if (coverPre.isEmpty()) {
        return;
    }
    for (const auto &nom : coverPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : coverPre) {
        nomNom(nom);
    }
    QString coverUrl =
        data.left(data.indexOf(coverPost.toUtf8())).replace("&amp;", "&") +
        "?s=512";
    if (coverUrl.left(4) != "http") {
        coverUrl.prepend(baseUrl + (coverUrl.left(1) == "/" ? "" : "/"));
    }
    netComm->request(coverUrl);
    q.exec();
    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        game.coverData = netComm->getData();
    }
}

void OpenRetro::getMarquee(GameEntry &game) {
    if (marqueePre.isEmpty()) {
        return;
    }
    for (const auto &nom : marqueePre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : marqueePre) {
        nomNom(nom);
    }
    QString marqueeUrl =
        data.left(data.indexOf(marqueePost.toUtf8())).replace("&amp;", "&") +
        "?s=512";
    if (marqueeUrl.left(4) != "http") {
        marqueeUrl.prepend(baseUrl + (marqueeUrl.left(1) == "/" ? "" : "/"));
    }
    netComm->request(marqueeUrl);
    q.exec();
    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        game.marqueeData = netComm->getData();
    }
}

QList<QString> OpenRetro::getSearchNames(const QFileInfo &info,
                                         QString &debug) {
    const QString baseName = info.completeBaseName();
    QList<QString> searchNames;
    QString searchName = baseName;

    debug.append("Base name: '" + baseName + "'\n");
    qDebug() << "baseName" << baseName;

    searchName = lookupSearchName(info, baseName, debug);
    qDebug() << "searchName, after lookup SearchName" << searchName;
    if (info.suffix() == "lha") {
        // Insert uuid from whdload_db.xml first
        if (!config->whdLoadMap[baseName].second.isEmpty()) {
            searchNames.prepend("/game/" + config->whdLoadMap[baseName].second);
        }
    }
    searchName = NameTools::getUrlQueryName(searchName, 2);
    qDebug() << "searchName, after UrlQueryname" << searchName;

    Q_ASSERT(!searchName.isEmpty());

    // Look for '_aga_' or '[aga]' with the last char optional
    if (info.suffix() == "lha" &&
        QRegularExpression("[_[]{1}(Aga|AGA)[_\\]]{0,1}")
            .match(baseName)
            .hasMatch()) {
        searchNames.append("/browse?q=" + searchName + "+aga");
    }
    searchNames.append("/browse?q=" + searchName);

    if (searchName.indexOf(":") != -1 || searchName.indexOf("-") != -1) {
        searchName = searchName.left(searchName.indexOf(":")).simplified();
        searchName = searchName.left(searchName.indexOf("-")).simplified();
        searchNames.append("/browse?q=" + searchName);
    }

    return searchNames;
}
