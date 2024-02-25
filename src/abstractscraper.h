/***************************************************************************
 *            abstractscraper.h
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

#ifndef ABSTRACTSCRAPER_H
#define ABSTRACTSCRAPER_H

#include "gameentry.h"
#include "netcomm.h"
#include "netmanager.h"
#include "settings.h"

#include <QEventLoop>
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QSettings>

class AbstractScraper : public QObject {
    Q_OBJECT

public:
    enum MatchType { ABSTRACT, MATCH_ONE, MATCH_MANY };

    AbstractScraper(Settings *config, QSharedPointer<NetManager> manager,
                    MatchType type = {ABSTRACT});
    virtual ~AbstractScraper();
    virtual void getGameData(GameEntry &game);
    virtual QList<QString> getSearchNames(const QFileInfo &info,
                                          QString &debug);
    virtual QString getCompareTitle(const QFileInfo &info);
    virtual void runPasses(QList<GameEntry> &gameEntries, const QFileInfo &info,
                           QString &output, QString &debug);

    int reqRemaining = -1;
    MatchType getType() const { return type; };

#ifdef TESTING
    QList<QString> getRegionPrios() { return regionPrios; }
    void detectRegionFromFilename(const QFileInfo &info);
#endif

protected:
    Settings *config;

    virtual void getSearchResults(QList<GameEntry> &gameEntries,
                                  QString searchName, QString platform);
    virtual void populateGameEntry(GameEntry &game);
    virtual void getDescription(GameEntry &game);
    virtual void getDeveloper(GameEntry &game);
    virtual void getPublisher(GameEntry &game);
    virtual void getPlayers(GameEntry &game);
    virtual void getAges(GameEntry &game);
    virtual void getTags(GameEntry &game);
    virtual void getRating(GameEntry &game);
    virtual void getReleaseDate(GameEntry &game);
    virtual void getCover(GameEntry &game);
    virtual void getScreenshot(GameEntry &game);
    virtual void getWheel(GameEntry &game);
    virtual void getMarquee(GameEntry &game);
    virtual void getTexture(GameEntry &game);
    virtual void getTitle(GameEntry &);
    virtual void getVideo(GameEntry &game);
    virtual void getManual(GameEntry &game) { (void)game; };

    virtual void nomNom(const QString nom, bool including = true);
    bool checkNom(const QString nom);

    virtual bool platformMatch(QString found, QString platform);
    virtual int getPlatformId(const QString);

    QString lookupSearchName(const QFileInfo &info, const QString &baseName,
                             QString &debug);

    MatchType type = {ABSTRACT};

    QList<int> fetchOrder;

    QByteArray data;

    QString baseUrl;
    QString searchUrlPre;
    QString searchUrlPost;

    QString searchResultPre;

    QList<QString> urlPre;
    QString urlPost;
    QList<QString> titlePre;
    QString titlePost;
    QList<QString> platformPre;
    QString platformPost;
    QList<QString> descriptionPre;
    QString descriptionPost;
    QList<QString> developerPre;
    QString developerPost;
    QList<QString> publisherPre;
    QString publisherPost;
    QList<QString> playersPre;
    QString playersPost;
    QList<QString> agesPre;
    QString agesPost;
    QList<QString> tagsPre;
    QString tagsPost;
    QList<QString> ratingPre;
    QString ratingPost;
    QList<QString> releaseDatePre;
    QString releaseDatePost;
    QList<QString> coverPre;
    QString coverPost;
    QList<QString> screenshotPre;
    QString screenshotPost;
    QString screenshotCounter;
    QList<QString> wheelPre;
    QString wheelPost;
    QList<QString> marqueePre;
    QString marqueePost;
    QList<QString> texturePre;
    QString texturePost;
    QList<QString> videoPre;
    QString videoPost;

    // This is used when file names have a region in them. The original
    // regionPrios is in Settings
    QList<QString> regionPrios;

    NetComm *netComm;
    QEventLoop q; // Event loop for use when waiting for data from NetComm.

private:
    QString lookupArcadeTitle(const QString &baseName);
#ifndef TESTING
    void detectRegionFromFilename(const QFileInfo &info);
#endif
    const inline QList<QPair<QString, QString>> regionMap() {
        // use list of pairs to maintain order
        return QList<QPair<QString, QString>>{
            QPair<QString, QString>("europe|(e)", "eu"),
            QPair<QString, QString>("usa|(u)", "us"),
            QPair<QString, QString>("world", "wor"),
            QPair<QString, QString>("japan|(j)", "jp"),
            QPair<QString, QString>("brazil", "br"),
            QPair<QString, QString>("korea", "kr"),
            QPair<QString, QString>("taiwan", "tw"),
            QPair<QString, QString>("france", "fr"),
            QPair<QString, QString>("germany", "de"),
            QPair<QString, QString>("italy", "it"),
            QPair<QString, QString>("spain", "sp"),
            QPair<QString, QString>("china", "cn"),
            QPair<QString, QString>("australia", "au"),
            QPair<QString, QString>("sweden", "se"),
            QPair<QString, QString>("canada", "ca"),
            QPair<QString, QString>("netherlands", "nl"),
            QPair<QString, QString>("denmark", "dk"),
            QPair<QString, QString>("asia", "asi")};
    }
};

#endif // ABSTRACTSCRAPER_H
