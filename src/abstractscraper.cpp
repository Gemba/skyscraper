/***************************************************************************
 *            abstractscraper.cpp
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

#include "abstractscraper.h"

#include "gameentry.h"
#include "nametools.h"
#include "platform.h"
#include "strtools.h"

#include <QDomDocument>
#include <QRegularExpression>

AbstractScraper::AbstractScraper(Settings *config,
                                 QSharedPointer<NetManager> manager,
                                 MatchType type)
    : config(config), type(type) {
    netComm = new NetComm(manager);
    connect(netComm, &NetComm::dataReady, &q, &QEventLoop::quit);
}

AbstractScraper::~AbstractScraper() { netComm->deleteLater(); }

void AbstractScraper::getSearchResults(QList<GameEntry> &gameEntries,
                                       QString searchName, QString platform) {
    netComm->request(searchUrlPre + searchName + searchUrlPost);
    q.exec();
    data = netComm->getData();

    GameEntry game;

    while (data.indexOf(searchResultPre.toUtf8()) != -1) {
        nomNom(searchResultPre);

        // Digest until url
        for (const auto &nom : urlPre) {
            nomNom(nom);
        }
        game.url = baseUrl + "/" + data.left(data.indexOf(urlPost.toUtf8()));

        // Digest until title
        for (const auto &nom : titlePre) {
            nomNom(nom);
        }
        game.title = data.left(data.indexOf(titlePost.toUtf8()));

        // Digest until platform
        for (const auto &nom : platformPre) {
            nomNom(nom);
        }
        game.platform = data.left(data.indexOf(platformPost.toUtf8()));

        if (platformMatch(game.platform, platform)) {
            gameEntries.append(game);
        }
    }
}

void AbstractScraper::getGameData(GameEntry &game) {
    netComm->request(game.url);
    q.exec();
    data = netComm->getData();
    // printf("URL IS: '%s'\n", game.url.toStdString().c_str());
    // printf("DATA IS:\n'%s'\n", data.data());
    populateGameEntry(game);
}

void AbstractScraper::getTitle(GameEntry &) {}

void AbstractScraper::populateGameEntry(GameEntry &game) {
    for (int t : fetchOrder) {
        switch (t) {
        case TITLE:
            getTitle(game);
            break;
        case DESCRIPTION:
            getDescription(game);
            break;
        case DEVELOPER:
            getDeveloper(game);
            break;
        case PUBLISHER:
            getPublisher(game);
            break;
        case PLAYERS:
            getPlayers(game);
            break;
        case AGES:
            getAges(game);
            break;
        case RATING:
            getRating(game);
            break;
        case TAGS:
            getTags(game);
            break;
        case RELEASEDATE:
            getReleaseDate(game);
            break;
        case COVER:
            if (config->cacheCovers) {
                getCover(game);
            }
            break;
        case SCREENSHOT:
            if (config->cacheScreenshots) {
                getScreenshot(game);
            }
            break;
        case WHEEL:
            if (config->cacheWheels) {
                getWheel(game);
            }
            break;
        case MARQUEE:
            if (config->cacheMarquees) {
                getMarquee(game);
            }
            break;
        case TEXTURE:
            if (config->cacheTextures) {
                getTexture(game);
            }
            break;
        case VIDEO:
            if (config->videos) {
                getVideo(game);
            }
            break;
        case MANUAL:
            if (config->manuals) {
                getManual(game);
            }
            break;
        default:;
        }
    }
}

// TODO: openretro and worldofspectrum
void AbstractScraper::getDescription(GameEntry &game) {
    if (descriptionPre.isEmpty()) {
        return;
    }
    for (const auto &nom : descriptionPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : descriptionPre) {
        nomNom(nom);
    }

    game.description = data.left(data.indexOf(descriptionPost.toUtf8()))
                           .replace("&lt;", "<")
                           .replace("&gt;", ">");
    game.description = game.description.replace("\\n", "\n");

    // Remove all html tags within description
    game.description = StrTools::stripHtmlTags(game.description);
}

// TODO: openretro and worldofspectrum
void AbstractScraper::getDeveloper(GameEntry &game) {
    for (const auto &nom : developerPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : developerPre) {
        nomNom(nom);
    }
    game.developer = data.left(data.indexOf(developerPost.toUtf8()));
}

// TODO: openretro and worldofspectrum
void AbstractScraper::getPublisher(GameEntry &game) {
    if (publisherPre.isEmpty()) {
        return;
    }
    for (const auto &nom : publisherPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : publisherPre) {
        nomNom(nom);
    }
    game.publisher = data.left(data.indexOf(publisherPost.toUtf8()));
}

// TODO: openretro and worldofspectrum
void AbstractScraper::getPlayers(GameEntry &game) {
    if (playersPre.isEmpty()) {
        return;
    }
    for (const auto &nom : playersPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : playersPre) {
        nomNom(nom);
    }
    game.players = data.left(data.indexOf(playersPost.toUtf8()));
}

// TODO: only for html scrape modules (currently none)
void AbstractScraper::getAges(GameEntry &game) {
    if (agesPre.isEmpty()) {
        return;
    }
    for (const auto &nom : agesPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : agesPre) {
        nomNom(nom);
    }
    game.ages = data.left(data.indexOf(agesPost.toUtf8()));
}

// TODO: openretro and worldofspectrum
void AbstractScraper::getTags(GameEntry &game) {
    if (tagsPre.isEmpty()) {
        return;
    }
    for (const auto &nom : tagsPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : tagsPre) {
        nomNom(nom);
    }
    game.tags = data.left(data.indexOf(tagsPost.toUtf8()));
}

// TODO: openretro and worldofspectrum
void AbstractScraper::getRating(GameEntry &game) {
    if (ratingPre.isEmpty()) {
        return;
    }
    for (const auto &nom : ratingPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : ratingPre) {
        nomNom(nom);
    }
    game.rating = data.left(data.indexOf(ratingPost.toUtf8()));
    bool toDoubleOk = false;
    double rating = game.rating.toDouble(&toDoubleOk);
    if (toDoubleOk) {
        game.rating = QString::number(rating / 5.0);
    } else {
        game.rating = "";
    }
}

// TODO: openretro and worldofspectrum
void AbstractScraper::getReleaseDate(GameEntry &game) {
    if (releaseDatePre.isEmpty()) {
        return;
    }
    for (const auto &nom : releaseDatePre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : releaseDatePre) {
        nomNom(nom);
    }
    game.releaseDate =
        data.left(data.indexOf(releaseDatePost.toUtf8())).simplified();
}

// TODO: openretro and worldofspectrum
void AbstractScraper::getCover(GameEntry &game) {
    if (coverPre.isEmpty()) {
        return;
    }
    for (const auto &nom : coverPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : coverPre) {
        nomNom(nom);
    }
    QString coverUrl =
        data.left(data.indexOf(coverPost.toUtf8())).replace("&amp;", "&");
    if (coverUrl.left(4) != "http") {
        coverUrl.prepend(baseUrl + (coverUrl.left(1) == "/" ? "" : "/"));
    }
    netComm->request(coverUrl);
    q.exec();
    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        game.coverData = netComm->getData();
    }
}

// TODO: openretro only
void AbstractScraper::getScreenshot(GameEntry &game) {
    if (screenshotPre.isEmpty()) {
        return;
    }
    // Check that we have enough screenshots
    int screens = data.count(screenshotCounter.toUtf8());
    if (screens >= 1) {
        for (int a = 0; a < screens - (screens / 2); a++) {
            for (const auto &nom : screenshotPre) {
                nomNom(nom);
            }
        }
        QString screenshotUrl = data.left(data.indexOf(screenshotPost.toUtf8()))
                                    .replace("&amp;", "&");
        if (screenshotUrl.left(4) != "http") {
            screenshotUrl.prepend(baseUrl +
                                  (screenshotUrl.left(1) == "/" ? "" : "/"));
        }
        netComm->request(screenshotUrl);
        q.exec();
        QImage image;
        if (netComm->getError() == QNetworkReply::NoError &&
            image.loadFromData(netComm->getData())) {
            game.screenshotData = netComm->getData();
        }
    }
}

// TODO: only for html scrape modules (currently none)
void AbstractScraper::getWheel(GameEntry &game) {
    if (wheelPre.isEmpty()) {
        return;
    }
    for (const auto &nom : wheelPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : wheelPre) {
        nomNom(nom);
    }
    QString wheelUrl =
        data.left(data.indexOf(wheelPost.toUtf8())).replace("&amp;", "&");
    if (wheelUrl.left(4) != "http") {
        wheelUrl.prepend(baseUrl + (wheelUrl.left(1) == "/" ? "" : "/"));
    }
    netComm->request(wheelUrl);
    q.exec();
    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        game.wheelData = netComm->getData();
    }
}

// TODO: openretro only
void AbstractScraper::getMarquee(GameEntry &game) {
    if (marqueePre.isEmpty()) {
        return;
    }
    for (const auto &nom : marqueePre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : marqueePre) {
        nomNom(nom);
    }
    QString marqueeUrl =
        data.left(data.indexOf(marqueePost.toUtf8())).replace("&amp;", "&");
    if (marqueeUrl.left(4) != "http") {
        marqueeUrl.prepend(baseUrl + (marqueeUrl.left(1) == "/" ? "" : "/"));
    }
    netComm->request(marqueeUrl);
    q.exec();
    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        game.marqueeData = netComm->getData();
    }
}

// TODO: only for html scrape modules (currently none)
void AbstractScraper::getTexture(GameEntry &game) {
    if (texturePre.isEmpty()) {
        return;
    }

    for (const auto &nom : texturePre) {
        if (!checkNom(nom)) {
            return;
        }
    }

    for (const auto &nom : texturePre) {
        nomNom(nom);
    }

    QString textureUrl =
        data.left(data.indexOf(texturePost.toUtf8())).replace("&amp;", "&");
    if (textureUrl.left(4) != "http") {
        textureUrl.prepend(baseUrl + (textureUrl.left(1) == "/" ? "" : "/"));
    }
    netComm->request(textureUrl);
    q.exec();
    QImage image;
    if (netComm->getError() == QNetworkReply::NoError &&
        image.loadFromData(netComm->getData())) {
        game.textureData = netComm->getData();
    }
}

// TODO: only for html scrape modules (currently none)
void AbstractScraper::getVideo(GameEntry &game) {
    if (videoPre.isEmpty()) {
        return;
    }
    for (const auto &nom : videoPre) {
        if (!checkNom(nom)) {
            return;
        }
    }
    for (const auto &nom : videoPre) {
        nomNom(nom);
    }
    QString videoUrl =
        data.left(data.indexOf(videoPost.toUtf8())).replace("&amp;", "&");
    if (videoUrl.left(4) != "http") {
        videoUrl.prepend(baseUrl + (videoUrl.left(1) == "/" ? "" : "/"));
    }
    netComm->request(videoUrl);
    q.exec();
    if (netComm->getError() == QNetworkReply::NoError) {
        game.videoData = netComm->getData();
        game.videoFormat = videoUrl.right(3);
    }
}

void AbstractScraper::nomNom(const QString nom, bool including) {
    data.remove(0, data.indexOf(nom.toUtf8()) + (including ? nom.length() : 0));
}

bool AbstractScraper::checkNom(const QString nom) {
    if (data.indexOf(nom.toUtf8()) != -1) {
        return true;
    }
    return false;
}

QString AbstractScraper::lookupArcadeTitle(const QString &baseName) {
    if (config->arcadePlatform) {
        return config->mameMap[baseName];
    }
    return "";
}

QString AbstractScraper::lookupSearchName(const QFileInfo &info,
                                          const QString &baseName,
                                          QString &debug) {
    QString searchName = baseName;
    if (!config->aliasMap[baseName].isEmpty()) {
        debug.append("'aliasMap.csv' entry found\n");
        QString aliasName = config->aliasMap[baseName];
        debug.append("Alias name: '" + aliasName + "'\n");
        searchName = aliasName;
    } else if (info.suffix() == "lha") {
        if (QString whdTitle = config->whdLoadMap[baseName].first;
            !whdTitle.isEmpty()) {
            debug.append("'whdload_db.xml' entry found\n");
            searchName = whdTitle;
            debug.append("Entry name: '" + searchName + "'\n");
        } else {
            searchName = NameTools::getNameWithSpaces(baseName);
        }
    } else if (config->platform == "scummvm") {
        searchName = NameTools::getScummName(info, baseName, config->scummIni);
    } else if (QString romTitle = lookupArcadeTitle(baseName);
               !romTitle.isEmpty()) {
        debug.append("'mameMap.csv' entry found\n");
        searchName = romTitle;
        debug.append("Entry name: '" + searchName + "'\n");
    }
    return searchName;
}

QList<QString> AbstractScraper::getSearchNames(const QFileInfo &info,
                                               QString &debug) {
    const QString baseName = info.completeBaseName();
    QList<QString> searchNames;
    QString searchName = baseName;

    debug.append("Base name: '" + baseName + "'\n");

    if (config->scraper != "import") {
        searchName = lookupSearchName(info, baseName, debug);
    }

    Q_ASSERT(!searchName.isEmpty());

    searchNames.append(NameTools::getUrlQueryName(searchName));

    // If search name has a subtitle, also search without subtitle
    if (searchName.contains(":") || searchName.contains(" - ")) {
        QString noSubtitle =
            searchName.left(searchName.indexOf(":")).simplified();
        noSubtitle = noSubtitle.left(noSubtitle.indexOf(" - ")).simplified();
        // Only add if longer than 3. We don't want to search for "the" for
        // instance
        if (noSubtitle.length() > 3)
            searchNames.append(NameTools::getUrlQueryName(noSubtitle));
    }

    // If the search name has a Roman numeral, also search for an integer
    // numeral version, vice-versa
    if (NameTools::hasRomanNumeral(searchName) ||
        NameTools::hasIntegerNumeral(searchName)) {
        if (NameTools::hasRomanNumeral(searchName)) {
            searchName = NameTools::convertToIntegerNumeral(searchName);
        } else {
            searchName = NameTools::convertToRomanNumeral(searchName);
        }
        searchNames.append(NameTools::getUrlQueryName(searchName));

        // If search name has a subtitle, also search without subtitle
        if (searchName.contains(":") || searchName.contains(" - ")) {
            QString noSubtitle =
                searchName.left(searchName.indexOf(":")).simplified();
            noSubtitle =
                noSubtitle.left(noSubtitle.indexOf(" - ")).simplified();
            if (noSubtitle.length() > 3) {
                // Only add if longer than 3. We don't want to search for "the"
                // for instance
                searchNames.append(NameTools::getUrlQueryName(noSubtitle));
            }
        }
    }

    return searchNames;
}

QString AbstractScraper::getCompareTitle(const QFileInfo &info) {
    const QString baseName = info.completeBaseName();
    QString compareTitle;

    if (!config->aliasMap[baseName].isEmpty()) {
        compareTitle = config->aliasMap[baseName];
    } else if (info.suffix() == "lha") {
        if (QString whdTitle = config->whdLoadMap[baseName].first;
            !whdTitle.isEmpty()) {
            compareTitle = whdTitle;
        } else {
            compareTitle = NameTools::getNameWithSpaces(baseName);
        }
    } else if (config->platform == "scummvm") {
        compareTitle =
            NameTools::getScummName(info, baseName, config->scummIni);
    } else if (QString romTitle = lookupArcadeTitle(baseName);
               !romTitle.isEmpty()) {
        compareTitle = romTitle;
    } else {
        compareTitle = baseName;
    }

    // Now create actual compareTitle
    compareTitle = compareTitle.replace("_", " ");
    compareTitle = StrTools::stripBrackets(compareTitle);

    QRegularExpressionMatch match;

    // Always move ", The" to the beginning of the name
    match = QRegularExpression(", [Tt]he").match(compareTitle);
    if (match.hasMatch()) {
        compareTitle = compareTitle.replace(match.captured(0), "")
                           .prepend(match.captured(0).right(3) + " ");
    }

    // Remove "vX.XXX" versioning string if one is found
    match = QRegularExpression(
                " v[.]{0,1}([0-9]{1}[0-9]{0,2}[.]{0,1}[0-9]{1,4}|[IVX]{1,5})$")
                .match(compareTitle);
    if (match.hasMatch() && match.capturedStart(0) != -1) {
        compareTitle = compareTitle.left(match.capturedStart(0)).simplified();
    }

    return compareTitle;
}

void AbstractScraper::detectRegionFromFilename(const QFileInfo &info) {
    const QString fn = info.fileName();
    if (int leftParPos = fn.indexOf("("); leftParPos != -1) {
        // Autodetect region and append to region priorities
        QString regionString = fn.mid(leftParPos, fn.length());
        QListIterator<QPair<QString, QString>> iter(regionMap());
        while (iter.hasNext()) {
            QPair<QString, QString> e = iter.next();
            QStringList keys = e.first.split("|");
            for (const auto &k : keys) {
                if (regionString.contains(k, Qt::CaseInsensitive)) {
                    // regionMap is sorted from bigger regions to smaller
                    // prepend() assures smaller regions get higher priority
                    regionPrios.prepend(e.second);
                    if (keys.size() > 1) {
                        // append only one: "europe" or "(e)"
                        break;
                    }
                }
            }
        }
    }
}

void AbstractScraper::runPasses(QList<GameEntry> &gameEntries,
                                const QFileInfo &info, QString &output,
                                QString &debug) {
    // Reset region priorities to original list from Settings
    regionPrios = config->regionPrios;
    if (config->region.isEmpty()) {
        detectRegionFromFilename(info);
    }

    QList<QString> searchNames;
    if (!config->searchName.isEmpty()) {
        // Add the string provided by "--query"
        searchNames.append(config->searchName);
    } else {
        searchNames = getSearchNames(info, debug);
    }

    if (searchNames.isEmpty()) {
        return;
    }

    if (config->verbosity >= 3) {
        int i = 0;
        for (const auto &sn : searchNames) {
            debug.append(QString("Search name #%1: '%2'\n")
                             .arg(QString::number(++i), sn));
        }
    }

    int pass = 0;
    for (const auto &sn : searchNames) {
        output.append("\033[1;35mPass " + QString::number(++pass) + "\033[0m ");
        getSearchResults(gameEntries, sn, config->platform);
        if (config->verbosity >= 3) {
            debug.append("Tried with: '" + sn + "'\n");
            debug.append("Platform: " + config->platform + "\n");
        }
        if (!gameEntries.isEmpty()) {
            break;
        }
    }
}

bool AbstractScraper::platformMatch(QString found, QString platform) {
    for (const auto &p : Platform::get().getAliases(platform)) {
        if (found.toLower() == p) {
            return true;
        }
    }
    return false;
}

int AbstractScraper::getPlatformId(const QString) { return -1; }
