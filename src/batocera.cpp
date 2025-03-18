/*
 *  This file is part of skyscraper.
 *  Copyright 2025 Gemba @ GitHub
 *
 *  skyscraper is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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

#include "batocera.h"

#include "emulationstation.h"
#include "gameentry.h"

#include <QDir>
#include <QStringBuilder>
#include <QStringList>

Batocera::Batocera() {
    // always on
    config->relativePaths = true;
}

inline const QString baseFolder() { return QString("/userdata/roms/"); }

QStringList Batocera::extraGamelistTags(bool isFolder) {
    (void)isFolder;
    // none, will be detected dynamically
    return QStringList();
}

QStringList Batocera::createEsVariantXml(const GameEntry &entry) {
    QStringList l;
    // at this point entry has already added the oldEntry elements and attribs
    QStringList elemNames =
        entry.extraTagNames(GameEntry::Format::BATOCERA, entry);
    // FIXME
    for (auto const &t : elemNames) {
        // if path-like info (do they have attribs?)
        // l.append(elem("thumbnail", entry.coverFile, addEmptyElem(), true));
        // ...
        // else
        // just dump with attribs (elem() method not usable)
    }
    return l;
}

void Batocera::preserveVariants(const GameEntry &oldEntry, GameEntry &entry) {
    QStringList elemNames =
        oldEntry.extraTagNames(GameEntry::Format::BATOCERA, oldEntry);
    for (const auto &t : elemNames) {
        // exclude the baseline elements are already excluded
        // add if not set in scraped/updated entry
        if (entry.getEsExtra(t).isEmpty()) {
            QPair<QString, QDomNamedNodeMap> p = oldEntry.getEsExtraAttribs(t);
            entry.setEsExtra(t, p.first, p.second);
        }
    }
}

QString Batocera::getInputFolder() { return baseFolder() % config->platform; }

QString Batocera::getGameListFolder() {
    return baseFolder() % config->platform;
}

QString Batocera::getMediaFolder() { return baseFolder() % config->platform; }
