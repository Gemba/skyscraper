/***************************************************************************
 *            gameentry.h
 *
 *  Wed Jun 14 12:00:00 CEST 2017
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

#ifndef GAMEENTRY_H
#define GAMEENTRY_H

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>

enum : int {
    DESCRIPTION = 0,
    DEVELOPER,
    PUBLISHER,
    PLAYERS,
    TAGS,
    RELEASEDATE,
    COVER,
    SCREENSHOT,
    VIDEO,
    RATING,
    WHEEL,
    MARQUEE,
    AGES,
    TITLE,
    TEXTURE,
    MANUAL
};

class GameEntry {
public:
    enum Format { RETROPIE, ESDE };

    GameEntry();

    void calculateCompleteness(bool videoEnabled = false, bool manualEnabled = false);
    int getCompleteness() const;
    void resetMedia();

    // textual data
    QString id = "";
    QString path = "";
    QString title = "";
    QString titleSrc = "";
    QString platform = "";
    QString platformSrc = "";
    QString description = "";
    QString descriptionSrc = "";
    QString releaseDate = "";
    QString releaseDateSrc = "";
    QString developer = "";
    QString developerSrc = "";
    QString publisher = "";
    QString publisherSrc = "";
    QString tags = "";
    QString tagsSrc = "";
    QString players = "";
    QString playersSrc = "";
    QString ages = "";
    QString agesSrc = "";
    QString rating = "";
    QString ratingSrc = "";

    // binary data
    QByteArray coverData = QByteArray();
    QString coverFile = "";
    QString coverSrc = "";
    QByteArray screenshotData = QByteArray();
    QString screenshotFile = "";
    QString screenshotSrc = "";
    QByteArray wheelData = QByteArray();
    QString wheelFile = "";
    QString wheelSrc = "";
    QByteArray marqueeData = QByteArray();
    QString marqueeFile = "";
    QString marqueeSrc = "";
    QByteArray textureData = QByteArray();
    QString textureFile = "";
    QString textureSrc = "";
    QByteArray videoData = QByteArray();
    QString videoFile = "";
    QString videoSrc = "";
    QByteArray manualData = QByteArray();
    QString manualFile = "";
    QString manualSrc = "";

    // internal
    int searchMatch = 0;
    QString cacheId = "";
    QString source = "";
    QString url = "";
    QString sqrNotes = "";
    QString parNotes = "";
    QString videoFormat = "";
    QString baseName = "";
    QString absoluteFilePath = "";
    bool found = true;

    // used by mobygames
    QByteArray miscData = "";

    // Holds EmulationStation (RetroPie and derivates) specific metadata
    // for preservation. (metadata = anything which is not scrapable)
    QMap<QString, QString> esExtras;

    bool isFolder = false;

    // AttractMode specific metadata for preservation
    // #Name;Title;Emulator;CloneOf;Year;Manufacturer;Category;Players;Rotation;Control;Status;DisplayCount;DisplayType;AltRomname;AltTitle;Extra;Buttons
    QString aMCloneOf = "";
    QString aMRotation = "";
    QString aMControl = "";
    QString aMStatus = "";
    QString aMDisplayCount = "";
    QString aMDisplayType = "";
    QString aMAltRomName = "";
    QString aMAltTitle = "";
    QString aMExtra = "";
    QString aMButtons = "";

    // Pegasus specific metadata for preservation
    QList<QPair<QString, QString>> pSValuePairs;

    QString getEsExtra(const QString &tagName) const {
        return esExtras[tagName];
    };

    void setEsExtra(const QString &tagName, QString value) {
        esExtras[tagName] = value;
    };

    inline const QStringList extraTagNames(Format type, bool isFolder = false) {
        QStringList tagNames = {"favorite",   "hidden",  "playcount",
                                "lastplayed", "kidgame", "sortname"};
        if (type == Format::RETROPIE) {
            return tagNames;
        }
        tagNames +=
            {"collectionsortname", "completed",    "broken",     "nogamecount",
             "nomultiscrape",      "hidemetadata", "controller", "altemulator"};
        if (isFolder) {
            tagNames.append("folderlink");
        }
        return tagNames;
    };

private:
    double completeness = 0;
};

#endif // GAMEENTRY_H
