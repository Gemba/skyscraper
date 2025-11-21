/***************************************************************************
 *            abstractfrontend.h
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

#ifndef ABSTRACTFRONTEND_H
#define ABSTRACTFRONTEND_H

#include "gameentry.h"
#include "queue.h"
#include "settings.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QObject>
#include <QSharedPointer>

struct MediaProps {
    GameEntry::Types type;
    QByteArray *data;
    QString *file;
    bool skip;
    QString ext;

    MediaProps(GameEntry::Types type, QByteArray &data, QString &file,
               bool skip)
        : type(type), data(&data), file(&file), skip(skip), ext(""){};
};

class AbstractFrontend : public QObject {
    Q_OBJECT

public:
    AbstractFrontend();
    virtual ~AbstractFrontend();
    virtual void setConfig(Settings *config);
    virtual void checkReqs(){};
    virtual void assembleList(QString &, QList<GameEntry> &){};
    virtual void skipExisting(QList<GameEntry> &, QSharedPointer<Queue>){};
    virtual bool canSkip() { return false; };
    virtual bool loadOldGameList(const QString &) { return false; };
    virtual void preserveFromOld(GameEntry &){};
    virtual QString getGameListFileName() { return QString(); };
    virtual QString getInputFolder() { return QString(); };
    virtual QString getGameListFolder() { return QString(); };
    virtual QString getMediaFolder() { return QString(); };
    virtual QString getCoversFolder() { return QString(); };
    virtual QString getScreenshotsFolder() { return QString(); };
    virtual QString getWheelsFolder() { return QString(); };
    virtual QString getMarqueesFolder() { return QString(); };
    virtual QString getTexturesFolder() { return QString(); };
    virtual QString getVideosFolder() { return QString(); };
    virtual QString getManualsFolder() { return QString(); };
    virtual QString getFanartsFolder() { return QString(); };
    virtual QString getBackcoversFolder() { return QString(); };
    virtual void sortEntries(QList<GameEntry> &gameEntries);
    bool copyMedia(GameEntry::Types &, const QString &, const QString &,
                   GameEntry &);

protected:
    virtual QString getTargetFileName(GameEntry::Types t,
                                      const QString &baseName) {
        (void)t;
        return baseName;
    };
    virtual GameEntry::Types supportedMedia() {
        return GameEntry::Types(GameEntry::NONE);
    };
    virtual bool gamelistHasMediaPaths() { return true; };
    Settings *config;
    QList<GameEntry> oldEntries;
    QMimeDatabase mimeDb;

private:
    QString getTargetFilePath(GameEntry::Types t, const QString &baseName,
                              const QString &subPath, const QString &cacheFn,
                              QString ext = "");
    bool doCopy(GameEntry::Types t, const QString &src, QString &tgt,
                const QByteArray &data, bool skipExisting);
};

#endif // ABSTRACTFRONTEND_H
