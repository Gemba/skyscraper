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

#ifndef GAMEBASE_H
#define GAMEBASE_H

#include "abstractscraper.h"

#include <QSqlDatabase>
#include <QtSql>

class GamebaseScraper : public AbstractScraper {
    Q_OBJECT

public:
    GamebaseScraper(Settings *config, QSharedPointer<NetManager> manager);
    ~GamebaseScraper();

private:
    QList<QString> getSearchNames(const QFileInfo &info,
                                  QString &debug) override;
    void getSearchResults(QList<GameEntry> &gameEntries, QString searchName,
                          QString platform) override;
    void getGameData(GameEntry &game) override;
    void getCover(GameEntry &game) override;
    QByteArray loadImageData(const QString &subFolder, QString &fileName);

    QSqlDatabase db;

    bool searchNameIsFilename = false;
};

#endif // GAMEBASE_H
