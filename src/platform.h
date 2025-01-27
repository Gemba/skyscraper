/***************************************************************************
 *            platform.h
 *
 *  Sat Dec 23 10:00:00 CEST 2017
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

#ifndef PLATFORM_H
#define PLATFORM_H

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

class Platform : public QObject {
    Q_OBJECT
public:
    static Platform &get();

    bool loadConfig();
    bool isPlatformCfgfilePristine(const QString &platformCfgFilePath);
    void clearConfigData();

    QStringList getPlatforms() const;
    QString getFormats(QString platform, QString extensions,
                       QString addExtensions) const;
    QString getDefaultScraper() const;
    QStringList getAliases(QString platform) const;
    int getPlatformIdOnScraper(const QString platform,
                               const QString scraper) const;

private:
    bool loadPlatformsIdMap();
    bool parsePlatformsIdCsv(const QString &fnplatformsIdCsvFn);
    QJsonObject loadLocalConfig();

    QStringList platforms;
    QVariantMap peas;
    QMap<QString, QVector<int>> platformIdsMap;
};

#endif // PLATFORM_H
