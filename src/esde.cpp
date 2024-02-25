/*
 *  This file is part of skyscraper.
 *  Copyright 2024 Gemba @ GitHub
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

#include "esde.h"

#include "emulationstation.h"
#include "gameentry.h"

#include <QDir>
#include <QStringBuilder>
#include <QStringList>

Esde::Esde() {}

inline const QString baseFolder() {
    return QString(QDir::homePath() % "/ES-DE");
}

QStringList Esde::extraGamelistTags(bool isFolder) {
    GameEntry g;
    return g.extraTagNames(GameEntry::Format::ESDE, isFolder);
}

QStringList Esde::createEsVariantXml(const GameEntry &entry) {
    (void)entry;
    return QStringList();
}

QString Esde::getInputFolder() {
    return QDir::homePath() % "/ROMs/" % config->platform;
}

QString Esde::getGameListFolder() {
    return baseFolder() % "/gamelists/" % config->platform;
}

QString Esde::getMediaFolder() {
    return baseFolder() % "/downloaded_media/" % config->platform;
}
