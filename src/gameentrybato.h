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

#include <QStringList>

class GameEntryBato : GameEntry {
public:
    GameEntryBato();

    // FIXME
    inline const QStringList extraTagNames(bool isFolder = false) {
        QStringList tagNames = GameEntry::extraTagNames();

        tagNames +=
            {"collectionsortname", "completed",    "broken",     "nogamecount",
             "nomultiscrape",      "hidemetadata", "controller", "altemulator"};
        if (isFolder) {
            tagNames.append("folderlink");
        }
        return tagNames;
    };
};

#endif // GAMEENTRYBATO_H
