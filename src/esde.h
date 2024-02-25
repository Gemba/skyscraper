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

#ifndef ESDE_H
#define ESDE_H

#include "emulationstation.h"
#include "gameentry.h"

class Esde : public EmulationStation {
    Q_OBJECT

public:
    Esde();

    QString getInputFolder() override;
    QString getGameListFolder() override;
    QString getMediaFolder() override;

protected:
    QStringList createEsVariantXml(const GameEntry &entry);
    QStringList extraGamelistTags(bool isFolder);
    GameEntry::Format gamelistFormat() { return GameEntry::Format::ESDE; };
};

#endif // ESDE_H
