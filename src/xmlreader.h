/***************************************************************************
 *            xmlreader.h
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

#ifndef XMLREADER_H
#define XMLREADER_H

#include "gameentry.h"

#include <QDomDocument>

template <typename T>
class XmlReader : public QDomDocument {
public:
    XmlReader(const QString &inputFolder, const QStringList &gamelistExtraTags);
    ~XmlReader();
    bool setFile(QString filename);
    QList<T> getEntries();

private:
    void addEntries(const QDomNodeList &nodes, QList<T> &gameEntries,
                    bool isFolder = false);
    QString makeAbsolute(QString filePath);
    void addTextual(T &entry, const QDomNode &node);

    QString inputFolder;
    QStringList gamelistExtraTags;
};

#endif // XMLREADER_H
