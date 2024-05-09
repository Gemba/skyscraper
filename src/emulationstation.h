/***************************************************************************
 *            emulationstation.h
 *
 *  Wed Jun 18 12:00:00 CEST 2017
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

#ifndef EMULATIONSTATION_H
#define EMULATIONSTATION_H

#include "abstractfrontend.h"
#include "gameentry.h"
#include "platform.h"

#include <QRegularExpression>

class EmulationStation : public AbstractFrontend {
    Q_OBJECT

public:
    EmulationStation();

    void assembleList(QString &finalOutput,
                      QList<GameEntry> &gameEntries) override;
    bool skipExisting(QList<GameEntry> &gameEntries,
                      QSharedPointer<Queue> queue) override;
    bool canSkip() override;
    bool loadOldGameList(const QString &gameListFileString) override;
    void preserveFromOld(GameEntry &entry) override;

    QString getGameListFileName() override;
    QString getInputFolder() override;
    QString getGameListFolder() override;
    QString getCoversFolder() override;
    QString getScreenshotsFolder() override;
    QString getWheelsFolder() override;
    QString getMarqueesFolder() override;
    QString getTexturesFolder() override;
    QString getVideosFolder() override;
    QString getManualsFolder() override;

protected:
    virtual QStringList createEsVariantXml(const GameEntry &entry);
    virtual QStringList extraGamelistTags(bool isFolder /* ignored on RP ES */);
    virtual GameEntry::Format gamelistFormat() {
        return GameEntry::Format::RETROPIE;
    };

private:
    QString createXml(GameEntry &entry);
    bool isGameLauncher(QString &sub);
    void addFolder(QString &base, QString sub, QList<GameEntry> &added);
    bool existingInGamelist(GameEntry &entry);
    QString elem(const QString &elem, const QString &data, bool addEmptyElem,
                 bool isPath = false);

    const inline QRegularExpression isoTimeRe() const {
        return QRegularExpression("(^$|T[0-9]{6}$)");
    }
    const inline QString platformFileExtensions() {
        return Platform::get().getFormats(config->platform, config->extensions,
                                          config->addExtensions);
    }
};

#endif // EMULATIONSTATION_H
