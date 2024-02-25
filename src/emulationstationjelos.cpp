/***************************************************************************
 *            emulationstation.cpp
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

#include "emulationstationjelos.h"

#include "gameentry.h"
#include "platform.h"
#include "strtools.h"
#include "xmlreader.h"

#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QStringBuilder>

static inline QRegularExpression isoTimeRe() {
    return QRegularExpression("(^$|T[0-9]{6}$)");
}

void EmulationStationJelos::preserveFromOld(GameEntry &entry) {
    QString fn = getFilename(entry.path);
    for (const auto &oldEntry : oldEntries) {
        if (getFilename(oldEntry.path) == fn) {
            if (entry.eSFavorite.isEmpty()) {
                entry.eSFavorite = oldEntry.eSFavorite;
            }
            if (entry.eSHidden.isEmpty()) {
                entry.eSHidden = oldEntry.eSHidden;
            }
            if (entry.eSPlayCount.isEmpty()) {
                entry.eSPlayCount = oldEntry.eSPlayCount;
            }
            if (entry.eSLastPlayed.isEmpty()) {
                entry.eSLastPlayed = oldEntry.eSLastPlayed;
            }
            if (entry.eSKidGame.isEmpty()) {
                entry.eSKidGame = oldEntry.eSKidGame;
            }
            if (entry.eSSortName.isEmpty() || entry.isFolder) {
                entry.eSSortName = oldEntry.eSSortName;
            }
            if (entry.developer.isEmpty() || entry.isFolder) {
                entry.developer = oldEntry.developer;
            }
            if (entry.publisher.isEmpty() || entry.isFolder) {
                entry.publisher = oldEntry.publisher;
            }
            if (entry.players.isEmpty() || entry.isFolder) {
                entry.players = oldEntry.players;
            }
            if (entry.description.isEmpty() || entry.isFolder) {
                entry.description = oldEntry.description;
            }
            if (entry.rating.isEmpty() || entry.isFolder) {
                entry.rating = oldEntry.rating;
            }
            if (entry.releaseDate.isEmpty() || entry.isFolder) {
                entry.releaseDate = oldEntry.releaseDate;
            }
            if (entry.tags.isEmpty() || entry.isFolder) {
                entry.tags = oldEntry.tags;
            }
            if (entry.isFolder) {
                entry.title = oldEntry.title;
                entry.coverFile = oldEntry.coverFile;
                entry.screenshotFile = oldEntry.screenshotFile;
                entry.wheelFile = oldEntry.wheelFile;
                entry.marqueeFile = oldEntry.marqueeFile;
                entry.textureFile = oldEntry.textureFile;
                entry.videoFile = oldEntry.videoFile;
                entry.manualFile = oldEntry.manualFile;
            }
            break;
        }
    }
}

QString EmulationStationJelos::createXml(GameEntry &entry) {
    QStringList l;
    bool addEmptyElem = !entry.isFolder;

    QString entryType = QString(entry.isFolder ? "folder" : "game");
    l.append("  <" % entryType % ">");

    l.append(elem("path", entry.path, addEmptyElem));
    l.append(elem("name", entry.title, addEmptyElem));

    l.append(elem("thumbnail", entry.coverFile, addEmptyElem, true));
    l.append(elem("image", entry.screenshotFile, addEmptyElem, true));
    l.append(elem("marquee", entry.marqueeFile, addEmptyElem, true));
    l.append(elem("texture", entry.textureFile, addEmptyElem, true));

    QString vidFile = entry.videoFile;
    if (!config->videos) {
        vidFile = "";
    }
    l.append(elem("video", vidFile, addEmptyElem, true));

    QString manFile = entry.manualFile;
    if (!config->manuals) {
        manFile = "";
    }
    l.append(elem("manual", manFile, addEmptyElem, true));

    l.append(elem("rating", entry.rating, addEmptyElem));
    l.append(elem("desc", entry.description, addEmptyElem));

    QString released = entry.releaseDate;
    QRegularExpressionMatch m = isoTimeRe().match(released);
    if (!m.hasMatch()) {
        released = released % "T000000";
    }
    l.append(elem("releasedate", released, addEmptyElem));

    l.append(elem("developer", entry.developer, addEmptyElem));
    l.append(elem("publisher", entry.publisher, addEmptyElem));
    l.append(elem("genre", entry.tags, addEmptyElem));
    l.append(elem("players", entry.players, addEmptyElem));

    // non scraper elements
    l.append(elem("sortname", entry.eSSortName, false));
    l.append(elem("favorite", entry.eSFavorite, false));
    l.append(elem("hidden", entry.eSHidden, false));
    l.append(elem("lastplayed", entry.eSLastPlayed, false));
    l.append(elem("playcount", entry.eSPlayCount, false));

    QString kidGame = entry.eSKidGame;
    if (entry.eSKidGame.isEmpty() && entry.ages.toInt() >= 1 &&
        entry.ages.toInt() <= 10) {
        kidGame = "true";
    }
    l.append(elem("kidgame", kidGame, false));

    l.append("  </" % entryType % ">");
    l.removeAll("");

    return l.join("\n") % "\n";
}

QString EmulationStationJelos::getManualsFolder() {
    return config->mediaFolder % "/manuals";
}