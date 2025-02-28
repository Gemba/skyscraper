/***************************************************************************
 *            importscraper.cpp
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

#include "importscraper.h"

#include <QDir>
#include <QDomDocument>

ImportScraper::ImportScraper(Settings *config,
                             QSharedPointer<NetManager> manager)
    : AbstractScraper(config, manager, MatchType::MATCH_ONE) {
    fetchOrder.append(TITLE);
    fetchOrder.append(DEVELOPER);
    fetchOrder.append(PUBLISHER);
    fetchOrder.append(COVER);
    fetchOrder.append(SCREENSHOT);
    fetchOrder.append(WHEEL);
    fetchOrder.append(MARQUEE);
    fetchOrder.append(TEXTURE);
    fetchOrder.append(VIDEO);
    fetchOrder.append(MANUAL);
    fetchOrder.append(RELEASEDATE);
    fetchOrder.append(TAGS);
    fetchOrder.append(PLAYERS);
    fetchOrder.append(AGES);
    fetchOrder.append(RATING);
    fetchOrder.append(DESCRIPTION);

    covers = QDir(config->importFolder + "/covers", "*.*", QDir::Name,
                  QDir::Files | QDir::NoDotAndDotDot)
                 .entryInfoList();
    screenshots = QDir(config->importFolder + "/screenshots", "*.*", QDir::Name,
                       QDir::Files | QDir::NoDotAndDotDot)
                      .entryInfoList();
    wheels = QDir(config->importFolder + "/wheels", "*.*", QDir::Name,
                  QDir::Files | QDir::NoDotAndDotDot)
                 .entryInfoList();
    marquees = QDir(config->importFolder + "/marquees", "*.*", QDir::Name,
                    QDir::Files | QDir::NoDotAndDotDot)
                   .entryInfoList();
    textures = QDir(config->importFolder + "/textures", "*.*", QDir::Name,
                    QDir::Files | QDir::NoDotAndDotDot)
                   .entryInfoList();
    videos = QDir(config->importFolder + "/videos", "*.*", QDir::Name,
                  QDir::Files | QDir::NoDotAndDotDot)
                 .entryInfoList();
    manuals = QDir(config->importFolder + "/manuals", "*.*", QDir::Name,
                   QDir::Files | QDir::NoDotAndDotDot)
                  .entryInfoList();
    textual = QDir(config->importFolder + "/textual", "*.*", QDir::Name,
                   QDir::Files | QDir::NoDotAndDotDot)
                  .entryInfoList();
    loadDefinitions();
}

void ImportScraper::getGameData(GameEntry &game) {
    // Always reset game title at this point, to avoid saving the dummy title in
    // cache
    game.title = "";

    loadData();
    populateGameEntry(game);
}

void ImportScraper::runPasses(QList<GameEntry> &gameEntries,
                              const QFileInfo &info, QString &, QString &) {
    data = "";
    textualFile = "";
    screenshotFile = "";
    coverFile = "";
    wheelFile = "";
    marqueeFile = "";
    videoFile = "";
    manualFile = "";
    GameEntry game;
    bool any = checkType(info.completeBaseName(), textual, textualFile);
    any |= checkType(info.completeBaseName(), screenshots, screenshotFile);
    any |= checkType(info.completeBaseName(), covers, coverFile);
    any |= checkType(info.completeBaseName(), wheels, wheelFile);
    any |= checkType(info.completeBaseName(), marquees, marqueeFile);
    any |= checkType(info.completeBaseName(), textures, textureFile);
    any |= checkType(info.completeBaseName(), videos, videoFile);
    any |= checkType(info.completeBaseName(), manuals, manualFile);
    if (any) {
        game.title = info.completeBaseName();
        game.platform = config->platform;
        gameEntries.append(game);
    }
}

QString ImportScraper::getCompareTitle(const QFileInfo &info) {
    return info.completeBaseName();
}

QByteArray ImportScraper::readFile(const QString &fn) {
    QByteArray data = QByteArray();
    if (!fn.isEmpty()) {
        QFile f(fn);
        if (f.open(QIODevice::ReadOnly)) {
            data = f.readAll();
            f.close();
        }
    }
    return data;
}

void ImportScraper::getCover(GameEntry &game) {
    game.coverData = readFile(coverFile);
}

void ImportScraper::getScreenshot(GameEntry &game) {
    game.screenshotData = readFile(screenshotFile);
}

void ImportScraper::getWheel(GameEntry &game) {
    game.wheelData = readFile(wheelFile);
}

void ImportScraper::getMarquee(GameEntry &game) {
    game.marqueeData = readFile(marqueeFile);
}

void ImportScraper::getTexture(GameEntry &game) {
    game.textureData = readFile(textureFile);
}

void ImportScraper::getManual(GameEntry &game) {
    game.manualData = readFile(manualFile);
}

void ImportScraper::getVideo(GameEntry &game) {
    if (!videoFile.isEmpty()) {
        QFile f(videoFile);
        if (f.open(QIODevice::ReadOnly)) {
            QFileInfo i(videoFile);
            game.videoData = f.readAll();
            game.videoFormat = i.suffix();
            f.close();
        }
    }
}

void ImportScraper::getAges(GameEntry &game) {
    if (isXml) {
        game.ages = getElementText(agesPre);
    } else {
        QByteArray dataOrig = data;
        AbstractScraper::getAges(game);
        data = dataOrig;
    }
}

void ImportScraper::getDescription(GameEntry &game) {
    if (isXml) {
        game.description = getElementText(descriptionPre);
    } else {
        QByteArray dataOrig = data;
        AbstractScraper::getDescription(game);
        data = dataOrig;
    }
}

void ImportScraper::getDeveloper(GameEntry &game) {
    if (isXml) {
        game.developer = getElementText(developerPre);
    } else {
        QByteArray dataOrig = data;
        AbstractScraper::getDeveloper(game);
        data = dataOrig;
    }
}

void ImportScraper::getPlayers(GameEntry &game) {
    if (isXml) {
        game.players = getElementText(playersPre);
    } else {
        QByteArray dataOrig = data;
        AbstractScraper::getPlayers(game);
        data = dataOrig;
    }
}

void ImportScraper::getPublisher(GameEntry &game) {
    if (isXml) {
        game.publisher = getElementText(publisherPre);
    } else {
        QByteArray dataOrig = data;
        AbstractScraper::getPublisher(game);
        data = dataOrig;
    }
}

void ImportScraper::getRating(GameEntry &game) {
    if (ratingPre.isEmpty()) {
        return;
    }

    if (isXml) {
        game.rating = getElementText(ratingPre);
    } else {
        QByteArray dataOrig = data;
        for (const auto &nom : ratingPre) {
            if (!checkNom(nom)) {
                return;
            }
        }
        for (const auto &nom : ratingPre) {
            nomNom(nom);
        }
        game.rating = data.left(data.indexOf(ratingPost.toUtf8()));
        data = dataOrig;
    }

    // check for 0, 0.5, 1, 1.5, ... 5 (star rating)
    QRegularExpression re("^[0-5](\\.5)?$");
    QRegularExpressionMatch m = re.match(game.rating);
    if (m.hasMatch()) {
        double rating = game.rating.toDouble();
        if (rating <= 5.0) {
            game.rating = QString::number(rating / 5.0);
        } else {
            game.rating = "";
        }
        return;
    }

    // check for 0.0 ... 1.0 (decimal number)
    // known limitation: to yield 0.5 with this scale enter it as '.5' or '0.50'
    bool toDoubleOk = false;
    double rating = game.rating.toDouble(&toDoubleOk);
    if (toDoubleOk && rating >= 0.0 && rating <= 1.0) {
        game.rating = QString::number(rating);
    } else {
        game.rating = "";
    }
}

void ImportScraper::getReleaseDate(GameEntry &game) {
    if (isXml) {
        game.releaseDate = getElementText(releaseDatePre);
    } else {
        QByteArray dataOrig = data;
        AbstractScraper::getReleaseDate(game);
        data = dataOrig;
    }
}

void ImportScraper::getTags(GameEntry &game) {
    if (isXml) {
        game.tags = getElementText(tagsPre);
    } else {
        QByteArray dataOrig = data;
        AbstractScraper::getTags(game);
        data = dataOrig;
    }
}

void ImportScraper::getTitle(GameEntry &game) {
    if (titlePre.isEmpty()) {
        return;
    }
    if (isXml) {
        game.title = getElementText(titlePre);
    } else {
        QByteArray dataOrig = data;
        for (const auto &nom : titlePre) {
            if (!checkNom(nom)) {
                return;
            }
        }
        for (const auto &nom : titlePre) {
            nomNom(nom);
        }
        game.title = data.left(data.indexOf(titlePost.toUtf8())).simplified();
        data = dataOrig;
    }
}

QString ImportScraper::getElementText(QStringList e) {
    QString v;
    if (!e.isEmpty()) {
        QString elem = e.at(0);
        QDomDocument doc;
        doc.setContent(data);
        QDomElement root = doc.documentElement();
        QDomNode n = root.namedItem(elem);
        if (!n.isNull()) {
            v = n.toElement().text();
        }
    }
    return v;
}

void ImportScraper::loadData() {
    if (!textualFile.isEmpty()) {
        QFile f(textualFile);
        if (f.open(QIODevice::ReadOnly)) {
            data = f.readAll();
            f.close();
        }
    }
}

bool ImportScraper::loadDefinitions() {
    // Check for textual resource file
    QFile defFile;
    if (QFile::exists(config->importFolder + "/definitions.dat")) {
        // check for per-platform folder definitions file
        defFile.setFileName(config->importFolder + "/definitions.dat");
    } else {
        defFile.setFileName(config->importFolder + "/../definitions.dat");
    }
    if (defFile.open(QIODevice::ReadOnly)) {
        while (!defFile.atEnd()) {
            QString line(defFile.readLine());
            isXml |= checkForTag(titlePre, titlePost, titleTag, line);
            isXml |=
                checkForTag(publisherPre, publisherPost, publisherTag, line);
            isXml |=
                checkForTag(developerPre, developerPost, developerTag, line);
            isXml |= checkForTag(playersPre, playersPost, playersTag, line);
            isXml |= checkForTag(agesPre, agesPost, agesTag, line);
            isXml |= checkForTag(ratingPre, ratingPost, ratingTag, line);
            isXml |= checkForTag(tagsPre, tagsPost, tagsTag, line);
            isXml |= checkForTag(releaseDatePre, releaseDatePost,
                                 releaseDateTag, line);
            isXml |= checkForTag(descriptionPre, descriptionPost,
                                 descriptionTag, line);
        }
        defFile.close();
        return true;
    }
    return false;
}

bool ImportScraper::checkForTag(QList<QString> &pre, QString &post,
                                QString &tag, QString &line) {
    bool isXml = false;
    if (line.indexOf(tag) != -1 && line.indexOf(tag) != 0) {
        QString preStr = line.left(line.indexOf(tag));
        QString ttmp = preStr.trimmed();
        isXml = ttmp.startsWith("<") && ttmp.endsWith(">");
        if (isXml) {
            pre.append(ttmp.replace("<", "").replace(">", ""));
        } else {
            pre.append(preStr);
        }
        post = line.right(line.length() - (line.indexOf(tag) + tag.length()));
    }
    return isXml;
}

bool ImportScraper::checkType(QString baseName, QList<QFileInfo> &infos,
                              QString &inputFile) {
    for (const auto &i : infos) {
        if (i.completeBaseName() == baseName) {
            inputFile = i.absoluteFilePath();
            return true;
        }
    }
    return false;
}
