/***************************************************************************
 *            worldofspectrum.h
 *
 *  Wed Jun 18 12:00:00 CEST 2017
 *  Copyright 2017 Lars Muldjord
 *
 *  04/2025: worldofspectrum.h -> zxinfodk.h
 *  Copyright 2025 Gemba @ GitHub
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

#ifndef ZXINFODK_H
#define ZXINFODK_H

#include "abstractscraper.h"
#include "gameentry.h"

#include <QJsonDocument>
#include <QJsonObject>

class ZxInfoDk : public AbstractScraper {
    Q_OBJECT

public:
    ZxInfoDk(Settings *config, QSharedPointer<NetManager> manager);

private:
    void getSearchResults(QList<GameEntry> &gameEntries, QString searchName,
                          QString platform) override;
    void getGameData(GameEntry &game) override;

    void getDeveloper(GameEntry &game) override;
    void getPlayers(GameEntry &game) override;
    void getAges(GameEntry &game) override;
    void getTags(GameEntry &game) override;
    void getRating(GameEntry &game) override;
    void getReleaseDate(GameEntry &game) override;
    void getCover(GameEntry &game) override;
    void getScreenshot(GameEntry &game) override;

    QString getQueryUrl(QString searchName);
    GameEntry createMinimumGameEntry(const QString &id,
                                     const QJsonObject &jsonGameObj,
                                     const QString &platform,
                                     bool filecheckQuery);
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;
};

#endif // ZXINFODK_H
