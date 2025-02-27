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

#include "gameentry.h"
#include "nametools.h"
#include "strtools.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>

XmlReader::XmlReader() {}

XmlReader::~XmlReader() {}

bool XmlReader::setFile(QString filename) {
    bool result = false;

    QFile f(filename);
    if (f.open(QIODevice::ReadOnly)) {
        if (setContent(f.readAll(), false)) {
            result = true;
        }
        f.close();
    }
    return result;
}

QList<GameEntry> XmlReader::getEntries(QString inputFolder,
                                       const QStringList &gamelistExtraTags) {
    QList<GameEntry> gameEntries;

    QDomNodeList gameNodes = elementsByTagName("game");
    QDomNodeList pathNodes = elementsByTagName("folder");

    addEntries(gameNodes, gameEntries, inputFolder, gamelistExtraTags);
    addEntries(pathNodes, gameEntries, inputFolder, gamelistExtraTags, true);

    return gameEntries;
}

void XmlReader::addEntries(const QDomNodeList &nodes,
                           QList<GameEntry> &gameEntries,
                           const QString &inputFolder,
                           const QStringList &gamelistExtraTags,
                           bool isFolder) {
    for (int a = 0; a < nodes.length(); ++a) {
        GameEntry entry;
        const QDomNode node = nodes.at(a);
        QString p = node.firstChildElement("path").text();
        if (isFolder) {
            // Workaround for EmulationStation 2.11.2rp and earlier: Element
            // <path> in <folder> when denoting a relative path is saved from ES
            // without trailing "./"
            QFileInfo pi(p);
            if (pi.isRelative() && !p.startsWith("./")) {
                p = "./" + p;
            }
        }
        entry.path = makeAbsolute(p, inputFolder);

        addTextual(entry, node);

        // only for ES
        entry.coverFile = makeAbsolute(
            node.firstChildElement("thumbnail").text(), inputFolder);
        entry.screenshotFile =
            makeAbsolute(node.firstChildElement("image").text(), inputFolder);
        entry.marqueeFile =
            makeAbsolute(node.firstChildElement("marquee").text(), inputFolder);
        entry.textureFile =
            makeAbsolute(node.firstChildElement("texture").text(), inputFolder);
        entry.videoFile =
            makeAbsolute(node.firstChildElement("video").text(), inputFolder);
        if (!entry.videoFile.isEmpty()) {
            entry.videoFormat = "fromxml";
        }
        entry.manualFile =
            makeAbsolute(node.firstChildElement("manual").text(), inputFolder);

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

QString XmlReader::makeAbsolute(QString filePath, const QString &inputFolder) {
    if (filePath.startsWith("./")) {
        filePath.remove(0, 1);
        filePath.prepend(inputFolder);
    }
    return filePath;
}
