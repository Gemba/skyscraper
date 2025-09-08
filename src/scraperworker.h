/***************************************************************************
 *            scraperworker.h
 *
 *  Wed Jun 7 12:00:00 CEST 2017
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

#ifndef SCRAPERWORKER_H
#define SCRAPERWORKER_H

#include "abstractscraper.h"
#include "cache.h"
#include "netmanager.h"
#include "queue.h"
#include "settings.h"

#include <QImage>
#include <QThread>

class ScraperWorker : public QObject {
    Q_OBJECT

public:
    ScraperWorker(QSharedPointer<Queue> queue, QSharedPointer<Cache> cache,
                  QSharedPointer<NetManager> manager, Settings config,
                  QString threadId);
    ~ScraperWorker();
    void run();
    bool forceEnd = false;

signals:
    void allDone();
    void entryReady(const GameEntry &entry, const QString &output,
                    const QString &debug);

private:
    enum MediaHint : unsigned char {
        VIDEO = 1,
        MANUAL = VIDEO << 1,
        FANART = VIDEO << 2
    };
    AbstractScraper *scraper;

    Settings config;

    QSharedPointer<Cache> cache;
    QSharedPointer<NetManager> manager;
    QSharedPointer<Queue> queue;

    QString platformOrig;
    QString threadId;

    unsigned int editDistance(const std::string &s1, const std::string &s2);

    GameEntry getBestEntry(const QList<GameEntry> &gameEntries,
                           QString compareTitle, int compareYear,
                           int &lowestDistance, QString &debug);
    GameEntry getEntryFromUser(const QList<GameEntry> &gameEntries,
                               const GameEntry &suggestedGame,
                               const QString &compareTitle,
                               int &lowestDistance);
    int getSearchMatch(const QString &title, const QString &compareTitle,
                       const int &lowestDistance);
    int getReleaseYear(const QString releaseDateString);

    bool limitReached(QString &output);
    void copyMedia(MediaHint mediaHint, const QString &completeBaseName,
                   const QString &subPath, GameEntry &game,
                   bool isBatocera = false);
    bool matchTitles(const QString &thiz, const QString &that);
    QList<QString> splitTitle(const QString &title);
    bool matchWords(const QList<QString> theseWords,
                    const QList<QString> thoseWords);
};

#endif // SCRAPERWORKER_H
