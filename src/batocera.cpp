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

#include <QDir>
#include <QStringBuilder>
#include <QStringList>

Batocera::Batocera() {}

// FIXME
inline const QString baseFolder() {
    return QString(QDir::homePath() % "/FIXME");
}

QStringList Batocera::extraGamelistTags(bool isFolder) {
    (void)isFolder;
    return QStringList();
}

QStringList Batocera::createEsVariantXml(const GameEntry &entry) {
    (void)entry;
    return QStringList();
}

void Batocera::preserveVariants(const GameEntry &oldEntry, GameEntry &entry) {
    // FIXME
}

// FIXME
QString Batocera::getInputFolder() {
    return QDir::homePath() % "/ROMs/" % config->platform;
}

QString Batocera::getGameListFolder() {
    return baseFolder() % "/gamelists/" % config->platform;
}

QString Batocera::getMediaFolder() {
    return baseFolder() % "/downloaded_media/" % config->platform;
}
