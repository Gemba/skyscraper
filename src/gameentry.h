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
#include <QDomNamedNodeMap>
#include <QFlags>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>

class GameEntry {
public:
    enum Elem : uint {
        NONE = 0x00,
        TITLE = 0x01,
        DESCRIPTION = 0x02,
        DEVELOPER = 0x04,
        PUBLISHER = 0x08,
        PLAYERS = 0x10,
        TAGS = 0x20,
        RELEASEDATE = 0x40,
        RATING = 0x80,
        AGES = 0x100,
        COVER = 0x200,
        SCREENSHOT = 0x400,
        WHEEL = 0x800,
        MARQUEE = 0x1000,
        TEXTURE = 0x2000,
        VIDEO = 0x4000,
        MANUAL = 0x8000,
        FANART = 0x10000,
        BACKCOVER = 0x20000,
        ALL = (BACKCOVER << 1) - 1
    };
    Q_DECLARE_FLAGS(Types, Elem)

    enum Format { RETROPIE, ESDE, BATOCERA, ATTRACT, PEGASUS };

    static constexpr GameEntry::Types MEDIA =
        GameEntry::Types(BACKCOVER | COVER | FANART | MANUAL | MARQUEE |
                         SCREENSHOT | TEXTURE | VIDEO | WHEEL);

    static constexpr GameEntry::Types IMAGE = GameEntry::Types(
        BACKCOVER | COVER | FANART | MARQUEE | SCREENSHOT | TEXTURE | WHEEL);

    static const QMap<Elem, QString> commonGamelistElems() {
        /* KEY, "gamelist XML element" */
        auto m = QMap<Elem, QString>{{COVER, "thumbnail"},
                                     {DESCRIPTION, "desc"},
                                     {DEVELOPER, "developer"},
                                     {PUBLISHER, "publisher"},
                                     {PLAYERS, "players"},
                                     {TAGS, "genre"},
                                     {RELEASEDATE, "releasedate"},
                                     {SCREENSHOT, "image"},
                                     {VIDEO, "video"},
                                     {RATING, "rating"},
                                     // ES, ES-DE, Bato: wheel usage via marquee
                                     {WHEEL, "wheel"},
                                     {MARQUEE, "marquee"},
                                     {AGES, "kidgame"},
                                     {TITLE, "name"},
                                     {TEXTURE, "texture"},
                                     // ES (variants), ES-DE and Batocera
                                     {MANUAL, "manual"},
                                     {FANART, "fanart"},
                                     // Batocera: Part of Gamelist
                                     // ES variants: maybe part of Gamelist
                                     // ES-DE: Only in filesystem
                                     {BACKCOVER, "boxback"}};
        return m;
    };

    static const QString getTag(GameEntry::Elem e) {
        QString elemName = commonGamelistElems()[e];
        return elemName;
    };

    GameEntry();

    void calculateCompleteness(bool videoEnabled, bool manualEnabled,
                               bool fanartEnabled, bool backcoverEnabled);
    int getCompleteness() const;
    void resetMedia();
    const QString getEsExtra(const QString &tagName) const;
    QPair<QString, QDomNamedNodeMap>
    getEsExtraAttribs(const QString &tagName) const;
    void setEsExtra(const QString &tagName, QString value,
                    QDomNamedNodeMap map = QDomNamedNodeMap());
    const QStringList extraTagNames(Format type, const GameEntry &ge) const;

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
    qint64 videoSize = 0;
    QByteArray videoData = QByteArray();
    QString videoFile = "";
    QString videoSrc = "";
    QByteArray manualData = QByteArray();
    QString manualFile = "";
    QString manualSrc = "";
    QByteArray fanartData = QByteArray();
    QString fanartFile = "";
    QString fanartSrc = "";
    QByteArray backcoverData = QByteArray();
    QString backcoverFile = "";
    QString backcoverSrc = "";

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

    // Holds EmulationStation (RetroPie and derivates) specific metadata
    // for preservation. (metadata = anything which is not scrapable)
    QMap<QString, QPair<QString, QDomNamedNodeMap>> esExtras;

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

private:
    const QStringList extraElemNames(Format type, bool isFolder) const;
    double completeness = 0.0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(GameEntry::Types)

#endif // GAMEENTRY_H
