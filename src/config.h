/*
 *  This file is part of skyscraper.
 *  Copyright 2023 Gemba @ GitHub
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

#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QObject>
#include <QString>

class Config : public QObject {
    Q_OBJECT

public:
    enum class FileOp { KEEP, OVERWRITE, CREATE_DIST };
    enum class SkyFolderType { CONFIG, CACHE, IMPORT, RESOURCE, REPORT, LOG };

    typedef QMap<SkyFolderType, QString> SkyFolders;

    static QString getSkyFolder(SkyFolderType type = SkyFolderType::CONFIG);
    static QString getRetropieVersion();

    void initSkyFolders();
    void setupUserConfig();
    void checkLegacyFiles();
    void copyFile(const QString &src, const QString &dest, bool isPristine,
                  FileOp fileOp = FileOp::OVERWRITE);
    int isPlatformCfgPristine(QString platformCfgFilePath);
    QString getSupportedPlatforms();

signals:
    void die(const int &, const QString &, const QString &);
};

#endif // CONFIG_H