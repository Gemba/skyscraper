/***************************************************************************
 *            xmlreader.cpp
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
#include "xmlreader.h"

#include "config.h"
#include "gameentry.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>

XmlReader::XmlReader(QString gameListFolder) {
    this->gameListFolder = gameListFolder;
}

XmlReader::~XmlReader() {}

bool XmlReader::setFile(QString filename) {
    bool result = false;

    QFile f(filename);
    if (f.open(QIODevice::ReadOnly)) {
#if QT_VERSION < 0x060800
        if (setContent(f.readAll(), false)) {
#else
        if (QDomDocument::ParseResult p = QDomDocument::setContent(f.readAll());
            p) {
#endif
            result = true;
        }
        f.close();
    }
    return result;
}

QList<GameEntry> XmlReader::getEntries(const QStringList &gamelistExtraTags) {
    QList<GameEntry> gameEntries;

    QDomNodeList gameNodes = elementsByTagName("game");
    QDomNodeList pathNodes = elementsByTagName("folder");

    addEntries(gameNodes, gameEntries, gamelistExtraTags);
    addEntries(pathNodes, gameEntries, gamelistExtraTags, true);

    return gameEntries;
}

void XmlReader::addEntries(const QDomNodeList &nodes,
                           QList<GameEntry> &gameEntries,
                           const QStringList &gamelistExtraTags,
                           bool isFolder) {
    for (int a = 0; a < nodes.length(); ++a) {
        GameEntry entry;
        const QDomNode node = nodes.at(a);

        entry.path = Config::makeAbsolutePath(
            gameListFolder, node.firstChildElement("path").text());

        addTextual(entry, node);

        // thumbnail only for ES
        entry.coverFile = Config::makeAbsolutePath(
            gameListFolder, node.firstChildElement("thumbnail").text());

        entry.screenshotFile = Config::makeAbsolutePath(
            gameListFolder, node.firstChildElement("image").text());
        entry.marqueeFile = Config::makeAbsolutePath(
            gameListFolder, node.firstChildElement("marquee").text());
        entry.textureFile = Config::makeAbsolutePath(
            gameListFolder, node.firstChildElement("texture").text());
        entry.videoFile = Config::makeAbsolutePath(
            gameListFolder, node.firstChildElement("video").text());
        if (!entry.videoFile.isEmpty()) {
            entry.videoFormat = "fromxml";
        }
        entry.manualFile = Config::makeAbsolutePath(
            gameListFolder, node.firstChildElement("manual").text());

        for (const auto &t : gamelistExtraTags) {
            entry.setEsExtra(t, node.firstChildElement(t).text());
        }

        entry.isFolder = isFolder;
        gameEntries.append(entry);
    }
}

void XmlReader::addTextual(GameEntry &entry, const QDomNode &node) {
    // Do NOT get sqr and par notes here. They are not used by skipExisting
    entry.title = node.firstChildElement("name").text();
    entry.description = node.firstChildElement("desc").text();
    entry.releaseDate = node.firstChildElement("releasedate").text();
    entry.developer = node.firstChildElement("developer").text();
    entry.publisher = node.firstChildElement("publisher").text();
    entry.tags = node.firstChildElement("genre").text();
    entry.rating = node.firstChildElement("rating").text();
    entry.players = node.firstChildElement("players").text();
}
