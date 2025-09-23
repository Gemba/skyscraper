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

XmlReader::XmlReader(QString inputFolder) { this->inputFolder = inputFolder; }

XmlReader::~XmlReader() {}

bool XmlReader::setFile(QString filename) {
    bool result = false;

    QFile f(filename);
    if (f.open(QIODevice::ReadOnly)) {
        QString eMsg;
        int eLine;
#if QT_VERSION < 0x060800
        if (setContent(f.readAll(), false, &eMsg, &eLine)) {
#else
        QDomDocument::ParseResult p = setContent(f.readAll());
        eMsg = p.errorMessage;
        eLine = p.errorLine;
        if (p) {
#endif
            result = true;
        } else {
            qWarning() << "XML error:" << eMsg << "at line" << eLine;
        }
        f.close();
    }
    return result;
}

QList<GameEntry> XmlReader::getEntries(const QStringList &gamelistExtraTags) {
    QList<GameEntry> gameEntries;

    QDomNodeList gameNodes = elementsByTagName("game");
    addEntries(gameNodes, gameEntries, gamelistExtraTags);

    QDomNodeList folderNodes = elementsByTagName("folder");
    addEntries(folderNodes, gameEntries, gamelistExtraTags, true);

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
            inputFolder, node.firstChildElement("path").text());

        addTextual(entry, node);

        entry.coverFile = Config::makeAbsolutePath(
            /* inputFolder is correct here as ES reads/expects relative media
               filepath in relation to the inputFolder */
            inputFolder,
            node.firstChildElement(GameEntry::getTag(GameEntry::Elem::COVER))
                .text());

        entry.screenshotFile = Config::makeAbsolutePath(
            inputFolder, node.firstChildElement(
                                 GameEntry::getTag(GameEntry::Elem::SCREENSHOT))
                             .text());
        entry.marqueeFile = Config::makeAbsolutePath(
            inputFolder,
            node.firstChildElement(GameEntry::getTag(GameEntry::Elem::MARQUEE))
                .text());
        entry.textureFile = Config::makeAbsolutePath(
            inputFolder,
            node.firstChildElement(GameEntry::getTag(GameEntry::Elem::TEXTURE))
                .text());
        entry.videoFile = Config::makeAbsolutePath(
            inputFolder,
            node.firstChildElement(GameEntry::getTag(GameEntry::Elem::VIDEO))
                .text());
        if (!entry.videoFile.isEmpty()) {
            entry.videoFormat = "fromxml";
        }
        entry.manualFile = Config::makeAbsolutePath(
            inputFolder,
            node.firstChildElement(GameEntry::getTag(GameEntry::Elem::MANUAL))
                .text());

        if (!gamelistExtraTags.isEmpty()) {
            entry.fanartFile = Config::makeAbsolutePath(
                inputFolder, node.firstChildElement(
                                     GameEntry::getTag(GameEntry::Elem::FANART))
                                 .text());
            // preserve these elements for ES and ES-DE
            for (const auto &t : gamelistExtraTags) {
                entry.setEsExtra(t, node.firstChildElement(t).text());
            }
        } else {
            // Bloatcera case
            if (node.hasAttributes()) {
                QDomNamedNodeMap attrs = node.attributes();
                QDomNode a = attrs.namedItem("id");
                if (!a.isNull()) {
                    QDomAttr attr = a.toAttr();
                    entry.id = attr.value();
                }
            }

            QDomNodeList elems = node.childNodes();
            for (int i = 0; i < elems.length(); i++) {
                QString k = elems.at(i).toElement().tagName();
                if (entry.commonGamelistElems().values().contains(k) ||
                    k == "path" ||
                    k == "sortname" /* when reading a non-batocera GL */) {
                    // it is a common/baseline gamelist element: do not keep in
                    // esExtra
                    continue;
                }
                // preserve everything else with attributes "as is"
                entry.setEsExtra(k, elems.at(i).toElement().text(),
                                 elems.at(i).toElement().attributes());
            }
        }
        entry.isFolder = isFolder;
        gameEntries.append(entry);
    }
}

void XmlReader::addTextual(GameEntry &entry, const QDomNode &node) {
    // Do NOT get sqr[] and par() notes here. They are not used by skipExisting
    entry.title = node.firstChildElement("name").text();
    entry.description =
        node.firstChildElement(GameEntry::getTag(GameEntry::Elem::DESCRIPTION))
            .text();
    entry.releaseDate =
        node.firstChildElement(GameEntry::getTag(GameEntry::Elem::RELEASEDATE))
            .text();
    entry.developer =
        node.firstChildElement(GameEntry::getTag(GameEntry::Elem::DEVELOPER))
            .text();
    entry.publisher =
        node.firstChildElement(GameEntry::getTag(GameEntry::Elem::PUBLISHER))
            .text();
    entry.tags =
        node.firstChildElement(GameEntry::getTag(GameEntry::Elem::TAGS)).text();
    entry.rating =
        node.firstChildElement(GameEntry::getTag(GameEntry::Elem::RATING))
            .text();
    entry.players =
        node.firstChildElement(GameEntry::getTag(GameEntry::Elem::PLAYERS))
            .text();
}
