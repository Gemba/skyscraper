/***************************************************************************
 *            worldofspectrum.cpp
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

#include "worldofspectrum.h"

#include "strtools.h"

WorldOfSpectrum::WorldOfSpectrum(Settings *config,
                                 QSharedPointer<NetManager> manager)
    : AbstractScraper(config, manager, MatchType::MATCH_MANY) {
    baseUrl = "https://www.worldofspectrum.org";

    searchResultPre =
        "<CENTER><FONT FACE=\"Arial,Helvetica\" COLOR=\"#000000\" SIZE=\"+1\">";
    urlPre.append("Full title");
    urlPre.append("<A HREF=\"");
    urlPost = "\" ";
    titlePre.append("TITLE=\"Get direct link to this entry\">");
    titlePost = "</A>";
    releaseDatePre.append("Year of release");
    releaseDatePre.append("<FONT FACE=");
    releaseDatePre.append("\">");
    releaseDatePost = "</FONT>";
    publisherPre.append("Find other titles from this publisher\">");
    publisherPost = "</A>";
    developerPre.append("Find other titles by this author\">");
    developerPost = "</A>";
    playersPre.append("Number of players");
    playersPre.append("<FONT");
    playersPre.append("\">");
    playersPost = "</FONT>";
    tagsPre.append("Type</FONT>");
    tagsPre.append("<FONT");
    tagsPre.append("\">");
    tagsPost = "</FONT>";
    descriptionPre.append("Additional info");
    descriptionPre.append("<FONT");
    descriptionPre.append("\">");
    descriptionPost = "</FONT>";
    coverPre.append("<TABLE BORDER=0 ALIGN=RIGHT>");
    coverPre.append("<TR><TD>");
    coverPost = "\" TARGET";
    screenshotPost = "\" BORDER";

    fetchOrder.append(COVER);
    fetchOrder.append(SCREENSHOT);
    fetchOrder.append(RELEASEDATE);
    fetchOrder.append(PUBLISHER);
    fetchOrder.append(DEVELOPER);
    fetchOrder.append(PLAYERS);
    fetchOrder.append(TAGS);
    fetchOrder.append(DESCRIPTION);
}

void WorldOfSpectrum::getSearchResults(QList<GameEntry> &gameEntries,
                                       QString searchName, QString platform) {
    searchName = searchName.replace("the+", "");
    netComm->request("https://www.worldofspectrum.org/infoseek.cgi",
                     "regexp=" + searchName + "&model=spectrum&loadpics=3");
    q.exec();
    data = netComm->getData();

    GameEntry game;

    while (data.indexOf(searchResultPre.toUtf8()) != -1) {
        nomNom(searchResultPre);

        // Digest until url
        for (const auto &nom : urlPre) {
            nomNom(nom);
        }
        game.url = baseUrl + data.left(data.indexOf(urlPost.toUtf8())) +
                   "&loadpics=3&allowadult=on";

        // Digest until title
        for (const auto &nom : titlePre) {
            nomNom(nom);
        }
        game.title = data.left(data.indexOf(titlePost.toUtf8()));
        if (game.title.contains(", The")) {
            game.title = game.title.replace(", The", "").prepend("The ");
        }

        game.platform = "zxspectrum";

        if (platformMatch(game.platform, platform)) {
            gameEntries.append(game);
        }
    }
}

void WorldOfSpectrum::getDescription(GameEntry &game) {
    for (const auto &nom : descriptionPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : descriptionPre) {
        nomNom(nom);
    }
    game.description = data.left(data.indexOf(descriptionPost.toUtf8()));
    // Remove all html tags within description
    game.description = StrTools::stripHtmlTags(game.description);
}

void WorldOfSpectrum::getCover(GameEntry &game) {
    for (const auto &nom : coverPre) {
        nomNom(nom);
    }
    if (data.left(2) != "<A") {
        return;
    }
    nomNom("<A HREF=\"");
    QString coverUrl = data.left(data.indexOf(coverPost.toUtf8()));
    if (coverUrl.indexOf("http") != -1) {
        game.coverData = downloadMedia(coverUrl);
    } else {
        game.coverData = downloadMedia(
            baseUrl + (coverUrl.left(1) == "/" ? "" : "/") + coverUrl);
    }
}

void WorldOfSpectrum::getScreenshot(GameEntry &game) {
    if (data.indexOf("<IMG SRC=\"/pub/sinclair/screens/in-game") == -1) {
        return;
    }
    nomNom("<IMG SRC=\"/pub/sinclair/screens/in-game", false);
    nomNom("<IMG SRC=\"");
    QString screenshotUrl = data.left(data.indexOf(screenshotPost.toUtf8()));
    if (screenshotUrl.indexOf("http") != -1) {
        game.screenshotData = downloadMedia(screenshotUrl);
    } else {
        game.screenshotData =
            downloadMedia(baseUrl + (screenshotUrl.left(1) == "/" ? "" : "/") +
                          screenshotUrl);
    }
}

void WorldOfSpectrum::getReleaseDate(GameEntry &game) {
    for (const auto &nom : releaseDatePre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : releaseDatePre) {
        nomNom(nom);
    }
    game.releaseDate = data.left(data.indexOf(releaseDatePost.toUtf8()));
    bool isInt = true;
    game.releaseDate.toInt(&isInt);
    if (!isInt) {
        game.releaseDate = "";
    }
}
