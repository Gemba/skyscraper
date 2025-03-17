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

#include <QDebug>
#include <QFile>
#include <QFileInfo>

template <typename T>
XmlReader<T>::XmlReader(const QString &inputFolder,
                        const QStringList &gamelistExtraTags) {
    this->inputFolder = inputFolder;
    this->gamelistExtraTags = gamelistExtraTags;
}

template <typename T>
XmlReader<T>::~XmlReader() {}

template <typename T>
bool XmlReader<T>::setFile(QString filename) {
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

template <typename T>
QList<T> XmlReader<T>::getEntries() {
    QList<T> gameEntries;

    QDomNodeList gameNodes = elementsByTagName("game");
    QDomNodeList pathNodes = elementsByTagName("folder");

    addEntries(gameNodes, gameEntries);
    addEntries(pathNodes, gameEntries, true);

    return gameEntries;
}

template <typename T>
void XmlReader<T>::addEntries(const QDomNodeList &nodes, QList<T> &gameEntries,
                              bool isFolder) {
    for (int a = 0; a < nodes.length(); ++a) {
        T entry;
        const QDomNode node = nodes.at(a);
        QString p = node.firstChildElement("path").text();
        if (isFolder) {
            // Workaround for EmulationStation 2.11.2rp and earlier: Element
            // <path> in <folder> when denoting a relative path is saved from ES
            // without heading "./"
            QFileInfo pi(p);
            if (pi.isRelative() && !p.startsWith("./")) {
                p = "./" + p;
            }
        }
        entry.path = makeAbsolute(p);

        addTextual(entry, node);

        entry.coverFile =
            makeAbsolute(node.firstChildElement("thumbnail").text());
        entry.screenshotFile =
            makeAbsolute(node.firstChildElement("image").text());
        entry.marqueeFile =
            makeAbsolute(node.firstChildElement("marquee").text());
        entry.textureFile =
            makeAbsolute(node.firstChildElement("texture").text());
        entry.videoFile = makeAbsolute(node.firstChildElement("video").text());
        if (!entry.videoFile.isEmpty()) {
            entry.videoFormat = "fromxml";
        }
        entry.manualFile =
            makeAbsolute(node.firstChildElement("manual").text());

        if (!gamelistExtraTags.isEmpty()) {
            // ES, ES-DE
            for (const auto &t : gamelistExtraTags)
                entry.setEsExtra(t, node.firstChildElement(t).text());
        } else {
            // Bloatcera
            QDomNodeList elems = node.childNodes();
            for (int i = 0; i < elems.length(); i++) {
                QString k = elems.at(i).toElement().tagName();
                if (entry.elements().values().contains(k)) {
                    // baseline gamelist element
                    continue;
                }
                entry.setEsExtra(k, elems.at(i).toElement().text(),
                                 elems.at(i).toElement().attributes());
            }
        }
        entry.isFolder = isFolder;
        gameEntries.append(entry);
    }
}

template <typename T>
void XmlReader<T>::addTextual(T &entry, const QDomNode &node) {
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

// FIXME: in util class
template <typename T>
QString XmlReader<T>::makeAbsolute(QString filePath) {
    if (filePath.startsWith("./")) {
        filePath.remove(0, 1);
        filePath.prepend(inputFolder);
    }
    return filePath;
}

// Let the linker know
template XmlReader<GameEntry>::XmlReader(const QString &inputFolder,
                                         const QStringList &gamelistExtraTags);
template XmlReader<GameEntry>::~XmlReader();
template QList<GameEntry> XmlReader<GameEntry>::getEntries();
template bool XmlReader<GameEntry>::setFile(QString);