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

#ifndef BATOCERA_H
#define BATOCERA_H

#include "emulationstation.h"
#include "gameentry.h"

#include <QDebug>

class Batocera : public EmulationStation {
    Q_OBJECT

public:
    Batocera();

    static QString getFileNameFor(const QString &resType,
                                  const QString &basename) {
        QString fnPostfix = getFilenamePostfix(resType);
        if (!fnPostfix.isEmpty()) {
            QString fn = QString("%1-%2").arg(basename).arg(fnPostfix);
            qDebug() << "lookup" << resType << "got filename" << fn;
            return fn;
        }
        qDebug() << "lookup" << resType << "returning input" << basename;
        return basename;
    }

    void setConfig(Settings *config) override;

    QString getInputFolder() override;
    QString getGameListFolder() override;
    QString getMediaFolder() override;
    QString getCoversFolder() override;
    QString getScreenshotsFolder() override;
    QString getWheelsFolder() override;
    QString getMarqueesFolder() override;
    QString getTexturesFolder() override;
    QString getFanartsFolder() override;
    QString getBackcoversFolder() override;

    QStringList extraGamelistTags(bool isFolder) override;

protected:
    QStringList createEsVariantXml(const GameEntry &entry) override;
    void preserveVariants(const GameEntry &oldEntry, GameEntry &entry) override;
    bool addEmptyElement() { return false; };
    QString openingElement(GameEntry &entry) override;
    QString getTargetFileName(GameEntry::Types t,
                              const QString &basename) override;
    GameEntry::Types supportedMedia() override;

private:
    static QString getFilenamePostfix(QString k) {
        QMap<QString, QString> cacheResFn = {
            // key: "binary type in cache", value: "-postfix"
            // extension will be derived from media data with Qt's mimedatabase
            {"backcover", "boxback"}, {"cover", "thumb"},
            {"fanart", "fanart"},     {"manual", "manual"},
            {"marquee", "marquee"},   {"screenshot", "image"},
            {"video", "video"},       {"wheel", "wheel"},
            // 'texture' not in Bato
        };
        return cacheResFn[k];
    }

    QString
    elemWithAttribs(const QString &t,
                    const QPair<QString, QDomNamedNodeMap> &elemAttribs);
};

#endif // BATOCERA_H
