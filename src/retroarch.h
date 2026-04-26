/*
 *  This file is part of skyscraper.
 *  Copyright 2026 SineSwiper @ GitHub
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

#ifndef RETROARCH_H
#define RETROARCH_H

#include "abstractfrontend.h"

class RetroArch : public AbstractFrontend {
    Q_OBJECT

public:
    RetroArch();
    void assembleList(QString &finalOutput,
                      QList<GameEntry> &gameEntries) override;
    void skipExisting(QList<GameEntry> &gameEntries,
                      QSharedPointer<Queue> queue) override;
    bool canSkip() override;
    bool loadOldGameList(const QString &gameListFileString) override;
    void preserveFromOld(GameEntry &entry) override;
    QString getGameListFileName() override;
    QString getInputFolder() override;
    QString getGameListFolder() override;
    QString getMediaFolder() override;
    QString getCoversFolder() override;
    QString getScreenshotsFolder() override;
    QString getMarqueesFolder() override;
    QString getWheelsFolder() override;

    GameEntry::Types supportedMedia() override {
        return GameEntry::Types(GameEntry::COVER | GameEntry::SCREENSHOT |
                                GameEntry::MARQUEE | GameEntry::WHEEL);
    }
    const QString getPlatformOutputName();

protected:
    // Override to use game title (sanitized) instead of ROM baseName for
    // thumbnail filenames
    QString getTargetFileName(GameEntry::Types t,
                              const QString &baseName) override;

    // RetroArch playlists do NOT embed media paths
    bool gamelistHasMediaPaths() override { return false; }

private:
    static QString sanitizeForFilename(const QString &name);
    static QString jsonEscape(const QString &str);

    QMap<QString, QString> baseNameToTitle;
    QMap<QString, QString> platformToDbName;
    static const QMap<QString, QString> platformToDbNameFallback;
};

#endif // RETROARCH_H
