/*
 *  This file is part of skyscraper.
 *  Copyright 2025 Gemba @ GitHub
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

#ifndef GAMEENTRYBATO_H
#define GAMEENTRYBATO_H

#include "gameentry.h"

#include <QDomNamedNodeMap>
#include <QStringList>

class GameEntryBato : GameEntry {
public:
    GameEntryBato();

    QPair<QString, QDomNamedNodeMap> getEsExtra(const QString &tagName) const {
        return esExtras[tagName];
    };

    void setEsExtra(const QString &tagName, const QString &value,
                    const QDomNamedNodeMap &attrMap) {
        esExtras[tagName] = QPair(value, attrMap);
    };

    inline const QStringList extraTagNames(bool isFolder = false) {
        (void)isFolder;
        return QStringList();
    };

private:
    QMap<QString, QPair<QString, QDomNamedNodeMap>> esExtras;
};

#endif // GAMEENTRYBATO_H
