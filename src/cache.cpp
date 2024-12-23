/***************************************************************************
 *            cache.cpp
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

#include "cache.h"

#include "cli.h"
#include "config.h"
#include "nametools.h"
#include "queue.h"
#include "skyscraper.h"

#include <QBuffer>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QStringBuilder>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>
#include <iostream>

// user defined resource cache entries
const auto SRC_USER = "user";

// quickids.xml and db.xml
const auto Q_ELEM = "quickid";
const auto R_ELEM = "resource";
const auto ATTR_FILEPATH = "filepath";
const auto ATTR_ID = "id";
const auto ATTR_SHA1_LEGACY = "sha1";
const auto ATTR_SRC = "source";
const auto ATTR_TS = "timestamp";
const auto ATTR_TYPE = "type";

static inline QStringList txtTypes(bool useGenres = true) {
    // keep order for cache edit menu
    QStringList txtTypes = {"title",     "platform", "releasedate", "developer",
                            "publisher", "players",  "ages"};
    txtTypes.append(useGenres ? "genres" : "tags");
    txtTypes += {"rating", "description"};
    return txtTypes;
}

static inline QStringList binTypes(bool withVideo = true,
                                   bool withManual = true) {
    // keep order for cache edit menu
    QStringList binTypes = {"cover", "screenshot", "wheel", "marquee",
                            "texture"};
    if (withVideo) {
        binTypes.append("video");
    }
    if (withManual) {
        binTypes.append("manual");
    }
    return binTypes;
};

const QStringList Cache::getAllResourceTypes() {
    return txtTypes() + binTypes();
}

Cache::Cache(const QString &cacheFolder) {
    cacheDir.setPath(cacheFolder);
    cacheDir.makeAbsolute();
    qDebug() << "Cache folder:" << cacheDir;
}

bool Cache::createFolders(const QString &scraper) {
    for (auto const &btype : binTypes()) {
        if (!cacheDir.mkpath(QString("%1/%2s/%3") // keep the plural 's'
                                 .arg(cacheDir.path(), btype, scraper))) {
            return false;
        }
    }

    // Copy priorities.xml example file to cache folder if it doesn't already
    // exist
    QFile::copy("cache/priorities.xml.example", prioFilePath());
    return true;
}

bool Cache::read() {
    QFile quickIdFile(quickIdFilePath());
    if (quickIdFile.open(QIODevice::ReadOnly)) {
        printf("Reading and parsing quick id xml, please wait... ");
        fflush(stdout);
        QXmlStreamReader xml(&quickIdFile);
        while (!xml.atEnd()) {
            if (xml.readNext() != QXmlStreamReader::StartElement) {
                continue;
            }
            if (xml.name() != Q_ELEM) {
                continue;
            }
            QXmlStreamAttributes attribs = xml.attributes();
            if (!attribs.hasAttribute(ATTR_FILEPATH) ||
                !attribs.hasAttribute(ATTR_TS) ||
                !attribs.hasAttribute(ATTR_ID)) {
                continue;
            }

            QPair<qint64, QString> pair;
            pair.first = attribs.value(ATTR_TS).toULongLong();
            pair.second = attribs.value(ATTR_ID).toString();
            quickIds[attribs.value(ATTR_FILEPATH).toString()] = pair;
        }
        printf("\033[1;32mDone!\033[0m\n");
    }

    QFile cacheFile(dbFilePath());
    if (cacheFile.open(QIODevice::ReadOnly)) {
        printf("Building file lookup cache, please wait... ");
        fflush(stdout);

        QSet<QString> fileEntries;
        for (auto const &t : binTypes()) {
            QDir dir(cacheDir.path() % "/" % t % "s", "*.*", QDir::Name,
                     QDir::Files);
            QDirIterator it(dir.absolutePath(),
                            QDir::Files | QDir::NoDotAndDotDot,
                            QDirIterator::Subdirectories);
            while (it.hasNext()) {
                fileEntries.insert(it.next());
            }
        }
        printf("\033[1;32mDone!\033[0m\n");
        printf("Cached %d files\n\n", fileEntries.count());

        printf("Reading and parsing resource cache, please wait... ");
        fflush(stdout);
        QXmlStreamReader xml(&cacheFile);
        while (!xml.atEnd()) {
            if (xml.readNext() != QXmlStreamReader::StartElement) {
                continue;
            }
            if (xml.name() != R_ELEM) {
                continue;
            }
            QXmlStreamAttributes attribs = xml.attributes();
            if (!attribs.hasAttribute(ATTR_SHA1_LEGACY) &&
                !attribs.hasAttribute(ATTR_ID)) {
                printf("Resource is missing unique id, skipping...\n");
                continue;
            }

            Resource resource;
            if (attribs.hasAttribute(
                    ATTR_SHA1_LEGACY)) { // Obsolete, but needed for backwards
                                         // compat
                resource.cacheId = attribs.value(ATTR_SHA1_LEGACY).toString();
            } else {
                resource.cacheId = attribs.value(ATTR_ID).toString();
            }

            if (attribs.hasAttribute(ATTR_SRC)) {
                resource.source = attribs.value(ATTR_SRC).toString();
            } else {
                resource.source = "generic";
            }
            if (attribs.hasAttribute(ATTR_TYPE)) {
                resource.type = attribs.value(ATTR_TYPE).toString();
                addToResCounts(resource.source, resource.type);
            } else {
                printf("Resource with cache id '%s' is missing 'type' "
                       "attribute, skipping...\n",
                       resource.cacheId.toStdString().c_str());
                continue;
            }
            if (attribs.hasAttribute(ATTR_TS)) {
                resource.timestamp = attribs.value(ATTR_TS).toULongLong();
            } else {
                printf("Resource with cache id '%s' is missing 'timestamp' "
                       "attribute, skipping...\n",
                       resource.cacheId.toStdString().c_str());
                continue;
            }
            resource.value = xml.readElementText();
            if (binTypes().contains(resource.type) &&
                !fileEntries.contains(cacheDir.path() % "/" % resource.value)) {
                printf("Source file '%s' missing, skipping entry...\n",
                       resource.value.toStdString().c_str());
                continue;
            }

            resources.append(resource);
        }
        cacheFile.close();
        resAtLoad = resources.length();
        printf("\033[1;32mDone!\033[0m\n");
        printf("Successfully parsed %d resources!\n\n", resources.length());
        return true;
    }
    return false;
}

void Cache::printPriorities(QString cacheId) {
    GameEntry game;
    game.cacheId = cacheId;
    fillBlanks(game);
    printf("\033[1;34mCurrent resource priorities for this rom:\033[0m\n");

    const QList<QPair<QString /*key*/,
                      QPair<QString /* resVal */, QString /* resSrc */>>>
        prioTxtRes = {{"Title", {game.title, game.titleSrc}},
                      {"Platform", {game.platform, game.platformSrc}},
                      {"Release Date", {game.releaseDate, game.releaseDateSrc}},
                      {"Developer", {game.developer, game.developerSrc}},
                      {"Publisher", {game.publisher, game.publisherSrc}},
                      {"Players", {game.players, game.playersSrc}},
                      {"Ages", {game.ages, game.agesSrc}},
                      {"Tags", {game.tags, game.tagsSrc}},
                      {"Rating", {game.rating, game.ratingSrc}}};

    const QString pad = "               ";
    for (auto const &e : prioTxtRes) {
        QString key = e.first;
        QString keyPadded =
            QString("%1:%2").arg(key, pad.left(pad.length() - key.length()));
        QString resVal = e.second.first;
        QString resSrc = e.second.second;
        if (resSrc.isEmpty()) {
            resSrc = QString("\033[1;31mmissing\033[0m");
        }
        printf("%s'\033[1;32m%s\033[0m' (%s)\n",
               keyPadded.toStdString().c_str(), resVal.toStdString().c_str(),
               resSrc.toStdString().c_str());
    }

    const QList<QPair<QString, QString>> prioBinRes = {
        {"Cover", game.coverSrc},     {"Screenshot", game.screenshotSrc},
        {"Wheel", game.wheelSrc},     {"Marquee", game.marqueeSrc},
        {"Texture", game.textureSrc}, {"Video", game.videoSrc},
        {"Manual", game.manualSrc}};

    for (auto const &e : prioBinRes) {
        QString key = QString("%1:%2").arg(
            e.first, pad.left(pad.length() - e.first.length()));
        printf("%s'", key.toStdString().c_str());
        if (e.second.isEmpty()) {
            printf("\033[1;31mNO\033[0m' ()\n");
        } else {
            printf("\033[1;32mYES\033[0m' (%s)\n",
                   e.second.toStdString().c_str());
        }
    }
    printf("Description:    (%s)\n'\033[1;32m%s\033[0m'",
           (game.descriptionSrc.isEmpty() ? QString("\033[1;31mmissing\033[0m")
                                          : game.descriptionSrc)
               .toStdString()
               .c_str(),
           game.description.toStdString().c_str());
    printf("\n\n");
}

void Cache::printCacheEditMenu() {
    printf("\033[1;34mWhat would you like to do?\033[0m\n"
           " Press Enter to continue to next rom in queue\n");
    printf("\033[1;33m  s\033[0m) Show current resource priorities for this "
           "rom\n");
    printf("\033[1;33m  S\033[0m) Show all cached resources for this rom\n");
    printf(
        "\033[1;33m  n\033[0m) Create new prioritized resource for this rom\n");
    printf("\033[1;33m  d\033[0m) Remove specific resource connected to this "
           "rom\n");
    printf(
        "\033[1;33m  D\033[0m) Remove ALL resources connected to this rom\n");
    printf("\033[1;33m  m\033[0m) Remove ALL resources connected to this rom "
           "from a specific module\n");
    printf("\033[1;33m  t\033[0m) Remove ALL resources connected to this rom "
           "of a specific type\n");
    printf("\033[1;33m  c\033[0m) Cancel all cache changes and exit\n");
    printf("\033[1;33m  q\033[0m) Save all cache changes and exit\n");
}

void Cache::editResources(QSharedPointer<Queue> queue, const QString &command,
                          const QString &type) {
    // Check sanity of command and parameters, if any
    if (!command.isEmpty()) {
        if (command == "new") {
            if (!txtTypes().contains(type)) {
                QStringList sortedTypes = txtTypes();
                sortedTypes.sort();
                printf("Unknown resource type '%s', please specify any of the "
                       "following: '%s'.\n",
                       type.toStdString().c_str(),
                       sortedTypes.join("', '").toStdString().c_str());
                return;
            }
        } else {
            printf("Unknown command '%s', please specify one of the following: "
                   "'new'.\n",
                   command.toStdString().c_str());
            return;
        }
    }

    int queueLength = queue->length();
    printf("\033[1;33mEntering resource cache editing mode! This mode allows "
           "you to edit textual resources for your files. To add media "
           "resources use the 'import' scraping module instead.\nNote that you "
           "can provide one or more file names on command line to edit "
           "resources for just those specific files. You can also use the "
           "'--startat' and '--endat' command line options to narrow down the "
           "span of the roms you wish to edit. Otherwise Skyscraper will edit "
           "ALL files found in the input folder one by one.\033[0m\n\n");
    while (queue->hasEntry()) {
        QFileInfo info = queue->takeEntry();
        QString cacheId = getQuickId(info);
        if (cacheId.isEmpty()) {
            cacheId = NameTools::getCacheId(info);
            addQuickId(info, cacheId);
        }
        bool doneEdit = false;
        printPriorities(cacheId);
        while (!doneEdit) {
            printf("\033[0;32m#%d/%d\033[0m \033[1;33m\nCURRENT FILE: "
                   "\033[0m\033[1;32m%s\033[0m\033[1;33m\033[0m\n",
                   queueLength - queue->length(), queueLength,
                   info.fileName().toStdString().c_str());
            std::string userInput = "";
            if (command.isEmpty()) {
                printCacheEditMenu();
                printf("> ");
                getline(std::cin, userInput);
                printf("\n");
            } else {
                if (command == "new") {
                    userInput = "n";
                    doneEdit = true;
                } else if (command == "quit") {
                    userInput = "q";
                }
            }
            if (userInput == "") {
                doneEdit = true;
                continue;
            } else if (userInput == "s") {
                printPriorities(cacheId);
            } else if (userInput == "S") {
                printf("\033[1;34mResources connected to this rom:\033[0m\n");
                bool found = false;
                for (const auto &res : resources) {
                    if (res.cacheId == cacheId) {
                        printf(
                            "\033[1;33m%s\033[0m (%s): '\033[1;32m%s\033[0m'\n",
                            res.type.toStdString().c_str(),
                            res.source.toStdString().c_str(),
                            res.value.toStdString().c_str());
                        found = true;
                    }
                }
                if (!found)
                    printf("None\n");
                printf("\n");
            } else if (userInput == "n") {
                GameEntry game;
                game.cacheId = cacheId;
                fillBlanks(game);
                std::string typeInput = "";
                if (type.isEmpty()) {
                    printf("\033[1;34mWhich resource type would you like to "
                           "create?\033[0m (Enter to cancel)\n");
                    const QList<QPair<QString, QString>> newResMenuItems = {
                        {"TItle", game.titleSrc},
                        {"Platform", game.platformSrc},
                        {"Release Date", game.releaseDateSrc},
                        {"Developer", game.developerSrc},
                        {"Publisher", game.publisherSrc},
                        {"Number of players", game.playersSrc},
                        {"Age rating", game.agesSrc},
                        {"Genres", game.tagsSrc},
                        {"Game rating", game.ratingSrc},
                        {"Description", game.descriptionSrc}};

                    int idx = 0;
                    for (auto const &e : newResMenuItems) {
                        const QString value =
                            (e.second.isEmpty() ? "(\033[1;31mmissing\033[0m)"
                                                : "");
                        printf("\033[1;33m %2d\033[0m) %s %s\n", idx++,
                               e.first.toStdString().c_str(),
                               value.toStdString().c_str());
                    }
                    printf("> ");
                    getline(std::cin, typeInput);
                    printf("\n");
                } else {
                    int idx = txtTypes().indexOf(type);
                    if (idx > -1) {
                        typeInput = QString::number(idx).toStdString().c_str();
                    }
                }
                if (typeInput == "") {
                    printf("Resource creation cancelled...\n\n");
                    continue;
                } else {
                    Resource newRes;
                    newRes.cacheId = cacheId;
                    newRes.source = SRC_USER;
                    newRes.timestamp =
                        QDateTime::currentDateTime().toMSecsSinceEpoch();
                    std::string valueInput = "";
                    // Default, matches everything except empty
                    QString expression = ".+";
                    bool ok;
                    QString tmpInput =
                        QString::fromUtf8(typeInput.data(), typeInput.size());
                    int tint = tmpInput.toInt(&ok);
                    if (!ok ||
                        (tint > txtTypes(false).length() - 1 || tint < 0)) {
                        printf("Invalid input, resource creation "
                               "cancelled...\n\n");
                        continue;
                    }
                    newRes.type = txtTypes(false)[tint];
                    if (tint == 0) {
                        printf("\033[1;34mPlease enter title:\033[0m (Enter to "
                               "cancel)\n> ");
                        getline(std::cin, valueInput);
                    } else if (tint == 1) {
                        printf("\033[1;34mPlease enter platform:\033[0m (Enter "
                               "to cancel)\n> ");
                        getline(std::cin, valueInput);
                    } else if (tint == 2) {
                        printf("\033[1;34mPlease enter a release date in the "
                               "format 'yyyy-MM-dd':\033[0m (Enter to "
                               "cancel)\n> ");
                        getline(std::cin, valueInput);
                        expression = "^[1-2]{1}[0-9]{3}-[0-1]{1}[0-9]{1}-[0-3]{"
                                     "1}[0-9]{1}$";
                    } else if (tint == 3) {
                        printf("\033[1;34mPlease enter developer:\033[0m "
                               "(Enter to cancel)\n> ");
                        getline(std::cin, valueInput);
                    } else if (tint == 4) {
                        printf("\033[1;34mPlease enter publisher:\033[0m "
                               "(Enter to cancel)\n> ");
                        getline(std::cin, valueInput);
                    } else if (tint == 5) {
                        printf(
                            "\033[1;34mPlease enter highest number of players "
                            "such as '4':\033[0m (Enter to cancel)\n> ");
                        getline(std::cin, valueInput);
                        expression = "^[0-9]{1,2}$";
                    } else if (tint == 6) {
                        printf("\033[1;34mPlease enter lowest age this should "
                               "be played at such as '10' which means "
                               "10+:\033[0m (Enter to cancel)\n> ");
                        getline(std::cin, valueInput);
                        expression = "^[0-9]{1}[0-9]{0,1}$";
                    } else if (tint == 7) {
                        printf("\033[1;34mPlease enter comma-separated genres "
                               "in the format 'Platformer, "
                               "Sidescrolling':\033[0m (Enter to cancel)\n> ");
                        getline(std::cin, valueInput);
                    } else if (tint == 8) {
                        printf("\033[1;34mPlease enter game rating from 0.0 to "
                               "1.0:\033[0m (Enter to cancel)\n> ");
                        getline(std::cin, valueInput);
                        expression = "^[0-1]{1}\\.{1}[0-9]{1}[0-9]{0,1}$";
                    } else if (tint == 9) {
                        printf(
                            "\033[1;34mPlease enter game description. Type "
                            "'\\n' for newlines:\033[0m (Enter to cancel)\n> ");
                        getline(std::cin, valueInput);
                    }
                    QString value = valueInput.c_str();
                    printf("\n");
                    value.replace("\\n", "\n");
                    if (valueInput == "") {
                        printf("Resource creation cancelled...\n\n");
                        continue;
                    } else if (!value.isEmpty() &&
                               QRegularExpression(expression)
                                   .match(value)
                                   .hasMatch()) {
                        newRes.value = value;
                        bool updated = false;
                        QMutableListIterator<Resource> it(resources);
                        while (it.hasNext()) {
                            Resource res = it.next();
                            if (res.cacheId == newRes.cacheId &&
                                res.type == newRes.type &&
                                res.source == newRes.source) {
                                it.remove();
                                updated = true;
                            }
                        }
                        resources.append(newRes);
                        if (updated) {
                            printf(">>> Updated existing ");
                        } else {
                            printf(">>> Added ");
                        }
                        printf("resource with value '\033[1;32m%s\033[0m'\n\n",
                               value.toStdString().c_str());
                        continue;
                    } else {
                        printf("\033[1;31mWrong format, resource hasn't been "
                               "added...\033[0m\n\n");
                        continue;
                    }
                }
            } else if (userInput == "d") {
                int rIdx = 0;
                QList<int> rIdxList;
                printf("\033[1;34mWhich resource id would you like to "
                       "remove?\033[0m (Enter to cancel)\n");
                for (const auto &res : resources) {
                    if (res.cacheId == cacheId &&
                        !binTypes().contains(res.type)) {
                        printf(
                            "\033[1;33m%4d\033[0m) \033[1;33m%s\033[0m (%s): "
                            "'\033[1;32m%s\033[0m'\n",
                            rIdxList.length() + 1,
                            res.type.toStdString().c_str(),
                            res.source.toStdString().c_str(),
                            res.value.toStdString().c_str());
                        rIdxList.append(rIdx);
                    }
                    rIdx++;
                }
                if (rIdxList.isEmpty()) {
                    printf("No resources found, cancelling...\n\n");
                    continue;
                }
                printf("> ");
                std::string typeInput = "";
                getline(std::cin, typeInput);
                printf("\n");
                if (typeInput == "") {
                    printf("Resource removal cancelled...\n\n");
                    continue;
                } else {
                    int chosen = atoi(typeInput.c_str());
                    if (chosen >= 1 && chosen <= rIdxList.length()) {
                        int index = rIdxList.at(chosen - 1);
                        QString delType = resources[index].type;
                        QString delSource = resources[index].source;
                        resources.removeAt(index);
                        printf("<<< Removed resource: %s (%s)\n\n",
                               delType.toStdString().c_str(),
                               delSource.toStdString().c_str());

                    } else {
                        printf("Invalid input, cancelling...\n\n");
                    }
                }
            } else if (userInput == "D") {
                QMutableListIterator<Resource> it(resources);
                bool found = false;
                while (it.hasNext()) {
                    Resource res = it.next();
                    if (res.cacheId == cacheId) {
                        printf("<<< Removed \033[1;33m%s\033[0m (%s) with "
                               "value '\033[1;32m%s\033[0m'\n",
                               res.type.toStdString().c_str(),
                               res.source.toStdString().c_str(),
                               res.value.toStdString().c_str());
                        it.remove();
                        found = true;
                    }
                }
                if (!found)
                    printf("No resources found for this rom...\n");
                printf("\n");
            } else if (userInput == "m") {
                printf("\033[1;34mResources from which module would you like "
                       "to remove?\033[0m (Enter to cancel)\n");
                QMap<QString, int> modules;
                for (const auto &res : resources) {
                    if (res.cacheId == cacheId) {
                        modules[res.source] += 1;
                    }
                }
                QMap<QString, int>::iterator it;
                for (it = modules.begin(); it != modules.end(); ++it) {
                    printf("'\033[1;33m%s\033[0m': %d resource(s) found\n",
                           it.key().toStdString().c_str(), it.value());
                }
                if (modules.isEmpty()) {
                    printf("No resources found, cancelling...\n\n");
                    continue;
                }
                printf("> ");
                std::string typeInput = "";
                getline(std::cin, typeInput);
                printf("\n");
                if (typeInput == "") {
                    printf("Resource removal cancelled...\n\n");
                    continue;
                } else if (modules.contains(QString(typeInput.c_str()))) {
                    QMutableListIterator<Resource> it(resources);
                    int removed = 0;
                    while (it.hasNext()) {
                        Resource res = it.next();
                        if (res.cacheId == cacheId &&
                            res.source == QString(typeInput.c_str())) {
                            it.remove();
                            removed++;
                        }
                    }
                    printf("<<< Removed %d resource(s) connected to rom from "
                           "module '\033[1;32m%s\033[0m'\n\n",
                           removed, typeInput.c_str());
                } else {
                    printf("No resources from module '\033[1;32m%s\033[0m' "
                           "found, cancelling...\n\n",
                           typeInput.c_str());
                }
            } else if (userInput == "t") {
                printf("\033[1;34mResources of which type would you like to "
                       "remove?\033[0m (Enter to cancel)\n");
                QMap<QString, int> types;
                for (const auto &res : resources) {
                    if (res.cacheId == cacheId) {
                        types[res.type] += 1;
                    }
                }
                QMap<QString, int>::iterator it;
                for (it = types.begin(); it != types.end(); ++it) {
                    printf("'\033[1;33m%s\033[0m': %d resource(s) found\n",
                           it.key().toStdString().c_str(), it.value());
                }
                if (types.isEmpty()) {
                    printf("No resources found, cancelling...\n\n");
                    continue;
                }
                printf("> ");
                std::string typeInput = "";
                getline(std::cin, typeInput);
                printf("\n");
                if (typeInput == "") {
                    printf("Resource removal cancelled...\n\n");
                    continue;
                } else if (types.contains(QString(typeInput.c_str()))) {
                    QMutableListIterator<Resource> it(resources);
                    int removed = 0;
                    while (it.hasNext()) {
                        Resource res = it.next();
                        if (res.cacheId == cacheId &&
                            res.type == QString(typeInput.c_str())) {
                            it.remove();
                            removed++;
                        }
                    }
                    printf("<<< Removed %d resource(s) connected to rom of "
                           "type '\033[1;32m%s\033[0m'\n\n",
                           removed, typeInput.c_str());
                } else {
                    printf("No resources of type '\033[1;32m%s\033[0m' found, "
                           "cancelling...\n\n",
                           typeInput.c_str());
                }
            } else if (userInput == "c") {
                printf("Exiting without saving changes.\n");
                exit(0);
            } else if (userInput == "q") {
                queue->clear();
                doneEdit = true;
                continue;
            }
        }
    }
}

bool Cache::purgeResources(QString purgeStr) {
    purgeStr.replace("purge:", "");
    printf("Purging requested resources from cache, please wait...\n");

    QString module = "";
    QString type = "";

    QList<QString> definitions = purgeStr.split(",");
    for (const auto &definition : definitions) {
        if (definition.left(2) == "m=") {
            module = definition.split("=").at(1).simplified();
            printf("Module: '%s'\n", module.toStdString().c_str());
        }
        if (definition.left(2) == "t=") {
            type = definition.split("=").at(1).simplified();
            printf("Type: '%s'\n", type.toStdString().c_str());
        }
    }

    int purged = 0;

    QMutableListIterator<Resource> it(resources);
    while (it.hasNext()) {
        Resource res = it.next();
        bool remove = false;
        if (res.source == module || res.type == type) {
            remove = true;
        }
        if (remove) {
            if (!removeMediaFile(res, "Couldn't purge media file '%s'")) {
                continue;
            }
            it.remove();
            purged++;
        }
    }
    printf("Successfully purged %d resources from the cache.\n", purged);
    return true;
}

bool Cache::purgeAll(const bool unattend) {
    if (!unattend) {
        printf("\033[1;31mWARNING! You are about to purge / remove ALL "
               "resources from the Skyscaper cache connected to the currently "
               "selected platform. THIS CANNOT BE UNDONE!\033[0m\n\n");
        printf("\033[1;34mDo you wish to continue\033[0m (y/N)? ");
        std::string userInput = "";
        getline(std::cin, userInput);
        if (userInput != "y") {
            printf("User chose not to continue, cancelling purge...\n\n");
            return false;
        }
    }

    printf("Purging ALL resources for %s platform, please wait...",
           cacheDir.dirName().toStdString().c_str());

    int purged = 0;
    int dots = 0;
    // Always make dotMod at least 1 or it will give "floating point exception"
    // when modulo
    int dotMod = resources.size() * 0.1 + 1;

    QMutableListIterator<Resource> it(resources);
    while (it.hasNext()) {
        if (dots % dotMod == 0) {
            printf(".");
            fflush(stdout);
        }
        dots++;
        Resource res = it.next();
        if (!removeMediaFile(res, "Couldn't purge media file '%s'")) {
            continue;
        }
        it.remove();
        purged++;
    }
    printf("\033[1;32m Done!\033[0m\n");
    if (purged == 0) {
        printf("No resources for the current platform found in the resource "
               "cache.\n");
        return false;
    } else {
        printf("Successfully purged %d resources from the resource cache.\n",
               purged);
    }
    printf("\n");
    return true;
}

bool Cache::isCommandValidOnAllPlatform(const QString &command) {
    QList<QString> validCommands({"help", "purge:all", "vacuum", "validate"});

    return validCommands.contains(command) ||
           command.contains("report:missing");
}

void Cache::purgeAllPlatform(Settings config, Skyscraper *app) {
    printf("\033[1;31mWARNING! You are about to purge / remove ALL "
           "resources from the Skyscaper cache. \033[0m\n\n");
    printf("\033[1;34mDo you wish to continue\033[0m (y/N)? ");
    std::string userInput = "";
    getline(std::cin, userInput);
    if (userInput != "y") {
        printf("User chose not to continue, cancelling purge...\n\n");
        return;
    }

    QDir cacheDir(config.cacheFolder);
    for (const auto &platform :
         cacheDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        config.platform = platform;
        Cache cache(cacheDir.filePath(platform));
        if (cache.read() && cache.purgeAll(true)) {
            app->state = Skyscraper::OpMode::NO_INTR;
            cache.write();
            app->state = Skyscraper::OpMode::SINGLE;
        }
    }
}

void Cache::reportAllPlatform(Settings config, Skyscraper *app) {
    QDir cacheDir(config.cacheFolder);
    for (const auto &platform :
         cacheDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        config.platform = platform;
        Cache cache(cacheDir.filePath(platform));
        if (cache.read()) {
            cache.assembleReport(config, app->getPlatformFileExtensions());
        }
    }
}

void Cache::vacuumAllPlatform(Settings config, Skyscraper *app) {
    printf("\033[1;33mWARNING! Vacuuming your Skyscraper cache removes all "
           "resources that don't match your current romset. Please consider "
           "making a backup of your "
           "Skyscraper cache before performing this action. THIS CANNOT BE "
           "UNDONE!\033[0m\n\n");
    printf("\033[1;34mDo you wish to continue\033[0m (y/N)? ");
    std::string userInput = "";
    getline(std::cin, userInput);
    if (userInput != "y") {
        printf("User chose not to continue, cancelling vacuum...\n\n");
        return;
    }

    QDir cacheDir(config.cacheFolder);
    for (const auto &platform :
         cacheDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        Cache cache(cacheDir.filePath(platform));
        config.platform = platform;
        if (cache.read() &&
            cache.vacuumResources(QDir(config.inputFolder).filePath(platform),
                                  app->getPlatformFileExtensions(),
                                  config.verbosity, true)) {
            app->state = Skyscraper::OpMode::NO_INTR;
            cache.write();
            app->state = Skyscraper::OpMode::SINGLE;
        }
    }
}

void Cache::validateAllPlatform(Settings config, Skyscraper *app) {
    QDir cacheDir(config.cacheFolder);
    for (const auto &platform :
         cacheDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        config.platform = platform;
        Cache cache(cacheDir.filePath(platform));
        if (cache.read()) {
            cache.validate();
            app->state = Skyscraper::OpMode::NO_INTR;
            cache.write();
            app->state = Skyscraper::OpMode::SINGLE;
        }
    }
}

QList<QFileInfo> Cache::getFileInfos(const QString &inputFolder,
                                     const QString &filter,
                                     const bool subdirs) {
    QList<QFileInfo> fileInfos;
    QStringList filters = filter.split(" ");
    if (filter.size() >= 2) {
        QDirIterator dirIt(inputFolder, filters,
                           QDir::Files | QDir::NoDotAndDotDot,
                           (subdirs ? QDirIterator::Subdirectories
                                    : QDirIterator::NoIteratorFlags));
        while (dirIt.hasNext()) {
            dirIt.next();
            fileInfos.append(dirIt.fileInfo());
        }
        if (fileInfos.isEmpty()) {
            printf("\nInput folder returned no entries...\n\n");
        }
    } else {
        printf("Found less than two suffix filters. Something is wrong...\n");
    }
    return fileInfos;
}

QList<QString> Cache::getCacheIdList(const QList<QFileInfo> &fileInfos) {
    QList<QString> cacheIdList;
    int dots = 0;
    // Always make dotMod at least 1 or it will give "floating point exception"
    // when modulo
    int dotMod = fileInfos.size() * 0.1 + 1;
    for (const auto &info : fileInfos) {
        if (dots % dotMod == 0) {
            printf(".");
            fflush(stdout);
        }
        dots++;
        QString cacheId = getQuickId(info);
        if (cacheId.isEmpty()) {
            cacheId = NameTools::getCacheId(info);
            addQuickId(info, cacheId);
        }
        cacheIdList.append(cacheId);
    }
    return cacheIdList;
}

void Cache::assembleReport(const Settings &config, const QString filter) {
    QString reportStr = config.cacheOptions;

    if (!reportStr.contains("report:missing=")) {
        printf("\033[1;31mAmbiguous cache report option '%s'.\n\033[0m",
               reportStr.toStdString().c_str());
        Cli::cacheReportMissingUsage();
        return;
    }
    reportStr.remove("report:missing=");

    QString missingOption = reportStr.simplified();
    QStringList resTypeList;
    if (missingOption.contains(",")) {
        resTypeList = missingOption.split(",");
    } else {
        if (missingOption == "all") {
            resTypeList += txtTypes(false); // contains 'tags' instead 'genres'
            resTypeList.sort();
            QStringList bt = binTypes();
            bt.sort();
            resTypeList += bt;
        } else if (missingOption == "textual") {
            resTypeList += txtTypes(false);
            resTypeList.sort();
        } else if (missingOption == "artwork") {
            resTypeList += binTypes(false, false); // w/o 'video' and 'manual'
            resTypeList.sort();
        } else if (missingOption == "media") {
            resTypeList += binTypes();
            resTypeList.sort();
        } else {
            resTypeList.append(missingOption); // If a single type is given
        }
    }
    for (const auto &resType : resTypeList) {
        if (!binTypes().contains(resType) &&
            !txtTypes(false).contains(resType)) {
            printf("\033[1;31mUnknown resource type '%s'!\033[0m\n",
                   resType.toStdString().c_str());
            Cli::cacheReportMissingUsage();
            return;
        }
    }
    if (resTypeList.isEmpty()) {
        printf("Resource type list is empty, cancelling...\n");
        return;
    } else {
        printf("Creating report(s) for resource type(s):\n");
        for (const auto &resType : resTypeList) {
            printf("  %s\n", resType.toStdString().c_str());
        }
        printf("\n");
    }

    // Create the reports folder
    QDir reportsDir(Config::getSkyFolder(Config::SkyFolderType::REPORT));
    if (!reportsDir.exists()) {
        if (!reportsDir.mkpath(".")) {
            printf("Couldn't create reports folder '%s'. Please check "
                   "permissions then try again...\n",
                   reportsDir.absolutePath().toStdString().c_str());
            return;
        }
    }

    Queue fileInfos;
    fileInfos.append(getFileInfos(config.inputFolder, filter, config.subdirs));
    if (!config.excludePattern.isEmpty()) {
        fileInfos.filterFiles(config.excludePattern);
    }
    if (!config.includePattern.isEmpty()) {
        fileInfos.filterFiles(config.includePattern, true);
    }
    printf("%d compatible files found for the '%s' platform!\n",
           fileInfos.length(), config.platform.toStdString().c_str());
    printf("Creating file id list for all files, please wait...");
    QList<QString> cacheIdList = getCacheIdList(fileInfos);
    printf("\n\n");

    if (fileInfos.length() != cacheIdList.length()) {
        printf("Length of cache id list mismatch the number of files, "
               "something is wrong! Please file an issue. Can't continue...\n");
        return;
    }

    QString dateTime = QDateTime::currentDateTime().toString("yyyyMMdd");
    for (const auto &resType : resTypeList) {
        QFile reportFile(reportsDir.absolutePath() + "/report-" +
                         config.platform + "-missing_" + resType + "-" +
                         dateTime + ".txt");
        printf("Report filename: '\033[1;32m%s\033[0m'\nAssembling report, "
               "please wait...",
               reportFile.fileName().toStdString().c_str());
        if (reportFile.open(QIODevice::WriteOnly)) {
            int missing = 0;
            int dots = 0;
            // Always make dotMod at least 1 or it will give "floating point
            // exception" when modulo
            int dotMod = fileInfos.size() * 0.1 + 1;

            for (int a = 0; a < fileInfos.length(); ++a) {
                if (dots % dotMod == 0) {
                    printf(".");
                    fflush(stdout);
                }
                dots++;
                bool found = false;
                for (const auto &res : resources) {
                    if (res.cacheId == cacheIdList.at(a) &&
                        res.type == resType) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    missing++;
                    reportFile.write(
                        fileInfos.at(a).absoluteFilePath().toUtf8() + "\n");
                }
            }
            reportFile.close();
            printf("\033[1;32m Done!\033[0m\n\033[1;33m%d file(s) is/are "
                   "missing the '%s' resource.\033[0m\n\n",
                   missing, resType.toStdString().c_str());
        } else {
            printf("Report file could not be opened for writing, please check "
                   "permissions of folder '%s', then try "
                   "again...\n",
                   Config::getSkyFolder(Config::SkyFolderType::REPORT)
                       .toStdString()
                       .c_str());
            return;
        }
    }
    printf("\033[1;32mAll done!\033[0m\nConsider using the '\033[1;33m--cache "
           "edit --includefrom <REPORTFILE>\033[0m' or the '\033[1;33m-s "
           "import\033[0m' module to add the missing resources. Check "
           "'\033[1;33m--help\033[0m' and '\033[1;33m--cache help\033[0m' for "
           "more information.\n\n");
}

bool Cache::vacuumResources(const QString inputFolder, const QString filter,
                            const int verbosity, const bool unattend) {
    if (!unattend) {
        std::string userInput = "";
        printf("\033[1;33mWARNING! Vacuuming your Skyscraper cache removes all "
               "resources that don't match your current romset (files located "
               "at '%s' or any of its subdirectories matching the suffixes "
               "supported by the platform and any extension(s) you might have "
               "added manually). Please consider making a backup of your "
               "Skyscraper cache before performing this action. The cache for "
               "this platform is listed under 'Cache folder' further up and is "
               "usually located under '%s' unless you've "
               "set it manually.\033[0m\n\n",
               inputFolder.toStdString().c_str(),
               Config::getSkyFolder(Config::SkyFolderType::CACHE)
                   .toStdString()
                   .c_str());
        printf("\033[1;34mDo you wish to continue\033[0m (y/N)? ");
        getline(std::cin, userInput);
        if (userInput != "y") {
            printf("User chose not to continue, cancelling vacuum...\n\n");
            return false;
        }
    }

    printf("Vacuuming cache for %s platform, this can take several minutes, "
           "please wait...",
           cacheDir.dirName().toStdString().c_str());
    QList<QFileInfo> fileInfos = getFileInfos(inputFolder, filter);
    // Clean the quick id's aswell
    QMap<QString, QPair<qint64, QString>> quickIdsCleaned;
    for (const auto &info : fileInfos) {
        QString filePath = info.absoluteFilePath();
        if (quickIds.contains(filePath)) {
            quickIdsCleaned[filePath] = quickIds[filePath];
        }
    }
    quickIds = quickIdsCleaned;
    QList<QString> cacheIdList = getCacheIdList(fileInfos);
    if (cacheIdList.isEmpty()) {
        printf("No cache id's found, something is wrong, cancelling...\n");
        return false;
    }

    int vacuumed = 0;
    {
        int dots = 0;
        // Always make dotMod at least 1 or it will give "floating point
        // exception" when modulo
        int dotMod = resources.size() * 0.1 + 1;

        QMutableListIterator<Resource> it(resources);
        while (it.hasNext()) {
            if (dots % dotMod == 0) {
                printf(".");
                fflush(stdout);
            }
            dots++;
            Resource res = it.next();
            bool remove = true;
            for (const auto &cacheId : cacheIdList) {
                if (res.cacheId == cacheId) {
                    remove = false;
                    break;
                }
            }
            if (remove) {
                if (!removeMediaFile(res, "Couldn't remove media file '%s'")) {
                    continue;
                }
                if (verbosity > 1)
                    printf("Purged resource for '%s' with value '%s'...\n",
                           res.cacheId.toStdString().c_str(),
                           res.value.toStdString().c_str());
                it.remove();
                vacuumed++;
            }
        }
    }
    printf("\033[1;32m Done!\033[0m\n");
    if (vacuumed == 0) {
        printf("All resources match a file in your romset. No resources "
               "vacuumed.\n");
        return false;
    } else {
        printf("Successfully vacuumed %d resources from the resource cache.\n",
               vacuumed);
    }
    printf("\n");
    return true;
}

void Cache::showStats(int verbosity) {
    printf("Resource cache stats for selected platform:\n");
    if (verbosity == 1) {
        printStats(true); /* totals */
    } else if (verbosity > 1) {
        printStats(false); /* per scrape module */
    }
    printf("\n");
}

void Cache::printStats(bool totals) {
    QMap<QString, int> resTotals = {
        {"Titles", 0},       {"Platforms", 0},  {"Descriptions", 0},
        {"Publishers", 0},   {"Developers", 0}, {"Players", 0},
        {"Ages", 0},         {"Tags", 0},       {"Ratings", 0},
        {"ReleaseDates", 0}, {"Covers", 0},     {"Screenshots", 0},
        {"Wheels", 0},       {"Marquees", 0},   {"Textures", 0},
        {"Videos", 0},       {"Manuals", 0}};
    for (auto it = resCountsMap.begin(); it != resCountsMap.end(); ++it) {
        if (!totals) {
            printf("'\033[1;32m%s\033[0m' module\n",
                   it.key().toStdString().c_str());
        }
        resTotals["Titles"] += it.value().titles;
        resTotals["Platforms"] += it.value().platforms;
        resTotals["Descriptions"] += it.value().descriptions;
        resTotals["Publishers"] += it.value().publishers;
        resTotals["Developers"] += it.value().developers;
        resTotals["Players"] += it.value().players;
        resTotals["Ages"] += it.value().ages;
        resTotals["Tags"] += it.value().tags;
        resTotals["Ratings"] += it.value().ratings;
        resTotals["ReleaseDates"] += it.value().releaseDates;
        resTotals["Covers"] += it.value().covers;
        resTotals["Screenshots"] += it.value().screenshots;
        resTotals["Wheels"] += it.value().wheels;
        resTotals["Marquees"] += it.value().marquees;
        resTotals["Textures"] += it.value().textures;
        resTotals["Videos"] += it.value().videos;
        resTotals["Manuals"] += it.value().manuals;
        if (!totals) {
            for (auto it = resTotals.begin(); it != resTotals.end(); ++it) {
                printf("  %12s : %d\n", it.key().toStdString().c_str(),
                       it.value());
                it.value() = 0;
            }
        }
    }
    if (totals) {
        for (auto it = resTotals.cbegin(); it != resTotals.cend(); ++it) {
            printf("  %12s : %d\n", it.key().toStdString().c_str(), it.value());
        }
    }
}

void Cache::addToResCounts(const QString source, const QString type) {
    if (type == "title") {
        resCountsMap[source].titles++;
    } else if (type == "platform") {
        resCountsMap[source].platforms++;
    } else if (type == "description") {
        resCountsMap[source].descriptions++;
    } else if (type == "publisher") {
        resCountsMap[source].publishers++;
    } else if (type == "developer") {
        resCountsMap[source].developers++;
    } else if (type == "players") {
        resCountsMap[source].players++;
    } else if (type == "ages") {
        resCountsMap[source].ages++;
    } else if (type == "tags") {
        resCountsMap[source].tags++;
    } else if (type == "rating") {
        resCountsMap[source].ratings++;
    } else if (type == "releasedate") {
        resCountsMap[source].releaseDates++;
    } else if (type == "cover") {
        resCountsMap[source].covers++;
    } else if (type == "screenshot") {
        resCountsMap[source].screenshots++;
    } else if (type == "wheel") {
        resCountsMap[source].wheels++;
    } else if (type == "marquee") {
        resCountsMap[source].marquees++;
    } else if (type == "texture") {
        resCountsMap[source].textures++;
    } else if (type == "video") {
        resCountsMap[source].videos++;
    } else if (type == "manual") {
        resCountsMap[source].manuals++;
    }
}

void Cache::readPriorities() {
    QDomDocument prioDoc;
    QFile prioFile(prioFilePath());
    printf("Looking for optional '\033[1;33mpriorities.xml\033[0m' file in "
           "cache folder... ");
    if (prioFile.open(QIODevice::ReadOnly)) {
        printf("\033[1;32mFound!\033[0m\n");
        if (!prioDoc.setContent(prioFile.readAll())) {
            printf("Document is not XML compliant, skipping...\n\n");
            return;
        }
    } else {
        printf("Not found, skipping...\n\n");
        return;
    }

    QDomNodeList orderNodes = prioDoc.elementsByTagName("order");

    int errors = 0;
    for (int a = 0; a < orderNodes.length(); ++a) {
        QDomElement orderElem = orderNodes.at(a).toElement();
        if (!orderElem.hasAttribute(ATTR_TYPE)) {
            printf("Priority 'order' node missing 'type' attribute, "
                   "skipping...\n");
            errors++;
            continue;
        }
        QString type = orderElem.attribute(ATTR_TYPE);
        QList<QString> sources;
        // ALWAYS prioritize 'user' resources highest (added with edit mode)
        sources.append(SRC_USER);
        QDomNodeList sourceNodes = orderNodes.at(a).childNodes();
        if (sourceNodes.isEmpty()) {
            printf("'source' node(s) missing for type '%s' in priorities.xml, "
                   "skipping...\n",
                   type.toStdString().c_str());
            errors++;
            continue;
        }
        for (int b = 0; b < sourceNodes.length(); ++b) {
            sources.append(sourceNodes.at(b).toElement().text());
        }
        prioMap[type] = sources;
    }
    printf("Priorities loaded successfully");
    if (errors != 0) {
        printf(", but %d error(s) encountered, please correct this", errors);
    }
    printf("!\n\n");
}

bool Cache::write(const bool onlyQuickId) {
    QMutexLocker locker(&cacheMutex);

    QFile quickIdFile(quickIdFilePath());
    if (quickIdFile.open(QIODevice::WriteOnly)) {
        printf("Writing quick id xml, please wait... ");
        fflush(stdout);
        QXmlStreamWriter xml(&quickIdFile);
        xml.setAutoFormatting(true);
        xml.writeStartDocument();
        xml.writeStartElement("quickids");
        for (const auto &key : quickIds.keys()) {
            xml.writeStartElement(Q_ELEM);
            xml.writeAttribute(ATTR_FILEPATH, key);
            xml.writeAttribute(ATTR_TS, QString::number(quickIds[key].first));
            xml.writeAttribute(ATTR_ID, quickIds[key].second);
            xml.writeEndElement();
        }
        xml.writeEndElement();
        xml.writeEndDocument();
        printf("\033[1;32mDone!\033[0m\n");
        quickIdFile.close();
        if (onlyQuickId) {
            return true;
        }
    }

    bool result = false;
    QFile cacheFile(dbFilePath());
    if (cacheFile.open(QIODevice::WriteOnly)) {
        printf("Writing %d (%d new) resources to cache, please wait... ",
               resources.length(), resources.length() - resAtLoad);
        fflush(stdout);
        QXmlStreamWriter xml(&cacheFile);
        xml.setAutoFormatting(true);
        xml.writeStartDocument();
        xml.writeStartElement("resources");
        for (const auto &resource : resources) {
            xml.writeStartElement(R_ELEM);
            xml.writeAttribute(ATTR_ID, resource.cacheId);
            xml.writeAttribute(ATTR_TYPE, resource.type);
            xml.writeAttribute(ATTR_SRC, resource.source);
            xml.writeAttribute(ATTR_TS, QString::number(resource.timestamp));
            xml.writeCharacters(resource.value);
            xml.writeEndElement();
        }
        xml.writeEndElement();
        xml.writeEndDocument();
        result = true;
        printf("\033[1;32mDone!\033[0m\n\n");
        cacheFile.close();
    }
    return result;
}

// This verifies all attached media files and deletes those that have no entry
// in the cache
void Cache::validate() {
    // TODO: Add format checks for each resource type, and remove if deemed
    // corrupt
    printf("Starting resource cache validation run for %s platform, please "
           "wait...\n",
           cacheDir.dirName().toStdString().c_str());

    if (!QFileInfo::exists(dbFilePath())) {
        printf("'db.xml' not found, cache cleaning cancelled...\n");
        return;
    }

    int filesDeleted = 0;
    int filesNoDelete = 0;

    for (auto const &t : binTypes()) {
        QDir dir(cacheDir.path() % "/" % t % "s", "*.*", QDir::Name,
                 QDir::Files);
        QDirIterator iter(dir.absolutePath(),
                          QDir::Files | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories);
        verifyFiles(iter, filesDeleted, filesNoDelete, t);
    }

    if (filesDeleted == 0 && filesNoDelete == 0) {
        printf("No inconsistencies found in the database. :)\n\n");
    } else {
        printf("Successfully deleted %d files with no resource entry.\n",
               filesDeleted);
        if (filesNoDelete != 0) {
            printf("%d files couldn't be deleted, please check file "
                   "permissions and re-run with '--cache validate'.\n",
                   filesNoDelete);
        }
        printf("\n");
    }
}

void Cache::verifyFiles(QDirIterator &dirIt, int &filesDeleted,
                        int &filesNoDelete, QString resType) {
    QList<QString> resFileNames;
    for (const auto &resource : resources) {
        if (resource.type == resType) {
            QFileInfo resInfo(cacheDir.path() + "/" + resource.value);
            resFileNames.append(resInfo.absoluteFilePath());
        }
    }

    while (dirIt.hasNext()) {
        QFileInfo fileInfo(dirIt.next());
        if (!resFileNames.contains(fileInfo.absoluteFilePath())) {
            printf("No resource entry for file '%s', deleting... ",
                   fileInfo.absoluteFilePath().toStdString().c_str());
            if (QFile::remove(fileInfo.absoluteFilePath())) {
                printf("OK!\n");
                filesDeleted++;
            } else {
                printf("ERROR! File couldn't be deleted :/\n");
                filesNoDelete++;
            }
        }
    }
}

void Cache::merge(Cache &mergeCache, bool overwrite,
                  const QString &mergeCacheFolder) {
    printf("Merging databases, please wait...\n");
    QList<Resource> mergeResources = mergeCache.getResources();

    QDir mergeCacheDir(mergeCacheFolder);
    mergeCacheDir.makeAbsolute();

    int resUpdated = 0;
    int resMerged = 0;

    for (const auto &mergeResource : mergeResources) {
        bool resExists = false;
        // This type of iterator ensures we can delete items while iterating
        QMutableListIterator<Resource> it(resources);
        while (it.hasNext()) {
            Resource res = it.next();
            if (res.cacheId == mergeResource.cacheId &&
                res.type == mergeResource.type &&
                res.source == mergeResource.source) {
                if (overwrite) {
                    if (!removeMediaFile(res,
                                         "Couldn't remove media file '%s' for "
                                         "updating")) {
                        continue;
                    }
                    it.remove();
                } else {
                    resExists = true;
                    break;
                }
            }
        }
        if (!resExists) {
            if (binTypes().contains(mergeResource.type)) {
                const QString absTgtFile =
                    cacheDir.path() + "/" + mergeResource.value;
                cacheDir.mkpath(absTgtFile);
                if (!QFile::copy(mergeCacheDir.path() + "/" +
                                     mergeResource.value,
                                 absTgtFile)) {
                    printf("Couldn't copy media file '%s', skipping...\n",
                           mergeResource.value.toStdString().c_str());
                    continue;
                }
            }
            if (overwrite) {
                resUpdated++;
            } else {
                resMerged++;
            }
            resources.append(mergeResource);
        }
    }
    printf("Successfully updated %d resource(s) in cache!\n", resUpdated);
    printf("Successfully merged %d new resource(s) into cache!\n\n", resMerged);
}

QList<Resource> Cache::getResources() { return resources; }

void Cache::addResources(GameEntry &entry, const Settings &config,
                         QString &output) {
    if (entry.source.isEmpty()) {
        printf("Something is wrong, resource with cache id '%s' has no source, "
               "exiting...\n",
               entry.cacheId.toStdString().c_str());
        exit(1);
    }
    if (entry.cacheId.isEmpty()) {
        return;
    }
    const QString cacheAbsolutePath = cacheDir.path();
    Resource resource;
    resource.cacheId = entry.cacheId;
    resource.source = entry.source;
    resource.timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    const QMap<QString, QString> txtResources = {
        {"title", entry.title},
        {"platform", entry.platform},
        {"description", entry.description},
        {"publisher", entry.publisher},
        {"developer", entry.developer},
        {"players", entry.players},
        {"ages", entry.ages},
        {"tags", entry.tags},
        {"rating", entry.rating},
        {"releasedate", entry.releaseDate}};

    for (auto e = txtResources.cbegin(), end = txtResources.cend(); e != end;
         ++e) {
        if (!e.value().isEmpty()) {
            resource.type = e.key();
            resource.value = e.value();
            addResource(resource, entry, cacheAbsolutePath, config, output);
        }
    }

    const QMap<QString, bool> binResources = {
        {"cover", !entry.coverData.isEmpty()},
        {"screenshot", !entry.screenshotData.isEmpty()},
        {"wheel", !entry.wheelData.isEmpty()},
        {"marquee", !entry.marqueeData.isEmpty()},
        {"texture", !entry.textureData.isEmpty()},
        {"manual", !entry.manualData.isEmpty()},
        {"video", !entry.videoData.isEmpty() && entry.videoFormat != ""}};

    for (auto const &t : binTypes()) {
        if (binResources.value(t)) {
            resource.type = t;
            resource.value = t % "s/" % entry.source % "/" % entry.cacheId;
            if (t == "video") {
                resource.value += "." % entry.videoFormat;
            }
            addResource(resource, entry, cacheAbsolutePath, config, output);
        }
    }
}

void Cache::addResource(Resource &resource, GameEntry &entry,
                        const QString &cacheAbsolutePath,
                        const Settings &config, QString &output) {
    QMutexLocker locker(&cacheMutex);
    bool notFound = true;
    // This type of iterator ensures we can delete items while iterating
    QMutableListIterator<Resource> it(resources);
    while (it.hasNext()) {
        Resource res = it.next();
        if (res.cacheId == resource.cacheId && res.type == resource.type &&
            res.source == resource.source) {
            if (config.refresh) {
                it.remove();
            } else {
                notFound = false;
            }
            break;
        }
    }

    if (notFound) {
        bool okToAppend = true;
        QString cacheFile = cacheAbsolutePath + "/" + resource.value;
        if (binTypes(false, false).contains(resource.type)) {
            QByteArray *imageData = nullptr;
            if (resource.type == "cover") {
                imageData = &entry.coverData;
            } else if (resource.type == "screenshot") {
                imageData = &entry.screenshotData;
            } else if (resource.type == "wheel") {
                imageData = &entry.wheelData;
            } else if (resource.type == "marquee") {
                imageData = &entry.marqueeData;
            } else if (resource.type == "texture") {
                imageData = &entry.textureData;
            }
            if (config.cacheResize) {
                QImage image;
                if (imageData->size() > 0 && image.loadFromData(*imageData) &&
                    !image.isNull()) {
                    int max = 800;
                    if (image.width() > max || image.height() > max) {
                        image = image.scaled(max, max, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
                    }
                    QByteArray resizedData;
                    QBuffer b(&resizedData);
                    b.open(QIODevice::WriteOnly);
                    if ((image.hasAlphaChannel() && hasAlpha(image)) ||
                        resource.type == "screenshot") {
                        okToAppend = image.save(&b, "png");
                    } else {
                        okToAppend = image.save(&b, "jpg", config.jpgQuality);
                    }
                    b.close();
                    if (imageData->size() > resizedData.size()) {
                        if (config.verbosity >= 3) {
                            printf("%s: '%d' > '%d', choosing resize for "
                                   "optimal result!\n",
                                   resource.type.toStdString().c_str(),
                                   imageData->size(), resizedData.size());
                        }
                        *imageData = resizedData;
                    }
                } else {
                    okToAppend = false;
                }
            }
            if (okToAppend) {
                QFile f(cacheFile);
                if (f.open(QIODevice::WriteOnly)) {
                    f.write(*imageData);
                    f.close();
                } else {
                    output.append("Error writing file: '" + f.fileName() +
                                  "' to cache. Please check permissions.");
                    okToAppend = false;
                }
            } else {
                // Image was faulty and could not be saved to cache so we clear
                // the QByteArray data in game entry to make sure we get a "NO"
                // in the terminal output from scraperworker.cpp.
                imageData->clear();
            }
        } else if (resource.type == "video") {
            if (entry.videoData.size() <= config.videoSizeLimit) {
                QFile f(cacheFile);
                if (f.open(QIODevice::WriteOnly)) {
                    f.write(entry.videoData);
                    f.close();
                    if (!config.videoConvertCommand.isEmpty()) {
                        output.append("Video conversion: ");
                        if (doVideoConvert(resource, cacheFile,
                                           cacheAbsolutePath, config, output)) {
                            output.append("\033[1;32mSuccess!\033[0m");
                        } else {
                            output.append(
                                "\033[1;31mFailed!\033[0m (set higher "
                                "'--verbosity N' level for more info)");
                            f.remove();
                            okToAppend = false;
                        }
                    }
                } else {
                    output.append("Error writing file: '" + f.fileName() +
                                  "' to cache. Please check permissions.");
                    okToAppend = false;
                }
            } else {
                output.append(
                    "Video exceeds maximum size of " +
                    QString::number(config.videoSizeLimit / 1000 / 1000) +
                    " MB. Adjust this limit with the 'videoSizeLimit' variable "
                    "in '" %
                        Config::getSkyFolder(Config::SkyFolderType::CONFIG) %
                        "/config.ini.'");
                okToAppend = false;
            }
        } else if (resource.type == "manual") {
            QFile f(cacheFile);
            if (f.open(QIODevice::WriteOnly)) {
                f.write(entry.manualData);
                f.close();
            } else {
                output.append("Error writing file: '" + f.fileName() +
                              "' to cache. Please check permissions.");
                okToAppend = false;
            }
        }

        if (okToAppend) {
            if (binTypes(false, false).contains(resource.type)) {
                // Remove old style cache image if it exists
                if (QFile::exists(cacheFile + ".png")) {
                    QFile::remove(cacheFile + ".png");
                }
            }
            resources.append(resource);
        } else {
            printf("\033[1;33mWarning! Couldn't add resource to cache. Have "
                   "you run out of disk space?\n\033[0m");
        }
    }
}

bool Cache::doVideoConvert(Resource &resource, QString &cacheFile,
                           const QString &cacheAbsolutePath,
                           const Settings &config, QString &output) {
    if (config.verbosity >= 2) {
        output.append("\n");
    }
    QString videoConvertCommand = config.videoConvertCommand;
    if (!videoConvertCommand.contains("%i")) {
        output.append(
            "'videoConvertCommand' is missing the required %i tag.\n");
        return false;
    }
    if (!videoConvertCommand.contains("%o")) {
        output.append(
            "'videoConvertCommand' is missing the required %o tag.\n");
        return false;
    }
    QFileInfo cacheFileInfo(cacheFile);
    QString tmpCacheFile = cacheFileInfo.absolutePath() + "/tmpfile_" +
                           (config.videoConvertExtension.isEmpty()
                                ? cacheFileInfo.fileName()
                                : cacheFileInfo.completeBaseName() + "." +
                                      config.videoConvertExtension);
    videoConvertCommand.replace("%i", cacheFile);
    videoConvertCommand.replace("%o", tmpCacheFile);
    if (QFile::exists(tmpCacheFile)) {
        if (!QFile::remove(tmpCacheFile)) {
            output.append("'" + tmpCacheFile +
                          "' already exists and can't be removed.\n");
            return false;
        }
    }
    if (config.verbosity >= 2) {
        output.append("%i: '" + cacheFile + "'\n");
        output.append("%o: '" + tmpCacheFile + "'\n");
    }
    if (config.verbosity >= 3) {
        output.append("Running command: '" + videoConvertCommand + "'\n");
    }
    QProcess convertProcess;
    if (videoConvertCommand.contains(" ")) {
        convertProcess.start(
            videoConvertCommand.split(' ').first(),
            QStringList({videoConvertCommand.split(' ').mid(1)}));
    } else {
        convertProcess.start(videoConvertCommand, QStringList({}));
    }
    // Wait 10 minutes max for conversion to complete
    if (convertProcess.waitForFinished(1000 * 60 * 10) &&
        convertProcess.exitStatus() == QProcess::NormalExit &&
        QFile::exists(tmpCacheFile)) {
        if (!QFile::remove(cacheFile)) {
            output.append("Original '" + cacheFile +
                          "' file couldn't be removed.\n");
            return false;
        }
        cacheFile = tmpCacheFile;
        cacheFile.replace("tmpfile_", "");
        if (QFile::exists(cacheFile)) {
            if (!QFile::remove(cacheFile)) {
                output.append("'" + cacheFile +
                              "' already exists and can't be removed.\n");
                return false;
            }
        }
        if (QFile::rename(tmpCacheFile, cacheFile)) {
            resource.value = cacheFile.replace(cacheAbsolutePath + "/", "");
        } else {
            output.append("Couldn't rename file '" + tmpCacheFile + "' to '" +
                          cacheFile + "', please check permissions!\n");
            return false;
        }
    } else {
        if (config.verbosity >= 3) {
            output.append(convertProcess.readAllStandardOutput() + "\n");
            output.append(convertProcess.readAllStandardError() + "\n");
        }
        return false;
    }
    if (config.verbosity >= 3) {
        output.append(convertProcess.readAllStandardOutput() + "\n");
        output.append(convertProcess.readAllStandardError() + "\n");
    }
    return true;
}

bool Cache::hasAlpha(const QImage &image) {
    QRgb *constBits = (QRgb *)image.constBits();
    for (int a = 0; a < image.width() * image.height(); ++a) {
        if (qAlpha(constBits[a]) < 127) {
            return true;
        }
    }
    return false;
}

void Cache::addQuickId(const QFileInfo &info, const QString &cacheId) {
    QMutexLocker locker(&quickIdMutex);
    QPair<qint64, QString> pair; // Quick id pair
    pair.first = info.lastModified().toMSecsSinceEpoch();
    pair.second = cacheId;
    quickIds[info.absoluteFilePath()] = pair;
}

QString Cache::getQuickId(const QFileInfo &info) {
    QMutexLocker locker(&quickIdMutex);
    if (quickIds.contains(info.absoluteFilePath()) &&
        info.lastModified().toMSecsSinceEpoch() <=
            quickIds[info.absoluteFilePath()].first) {
        return quickIds[info.absoluteFilePath()].second;
    }
    return QString();
}

bool Cache::hasEntries(const QString &cacheId, const QString scraper) {
    QMutexLocker locker(&cacheMutex);
    for (const auto &res : resources) {
        if ((scraper.isEmpty() || res.source == scraper) &&
            res.cacheId == cacheId) {
            return true;
        }
    }
    return false;
}

void Cache::fillBlanks(GameEntry &entry, const QString scraper) {
    QMutexLocker locker(&cacheMutex);
    QList<Resource> matchingResources;
    // Find all resources related to this particular rom
    for (const auto &resource : resources) {
        if ((scraper.isEmpty() || resource.source == scraper) &&
            entry.cacheId == resource.cacheId) {
            matchingResources.append(resource);
        }
    }

    for (auto type : txtTypes(false)) {
        QString result = "";
        QString source = "";
        if (fillType(type, matchingResources, result, source)) {
            if (type == "title") {
                entry.title = result;
                entry.titleSrc = source;
            } else if (type == "platform") {
                entry.platform = result;
                entry.platformSrc = source;
            } else if (type == "description") {
                entry.description = result;
                entry.descriptionSrc = source;
            } else if (type == "publisher") {
                entry.publisher = result;
                entry.publisherSrc = source;
            } else if (type == "developer") {
                entry.developer = result;
                entry.developerSrc = source;
            } else if (type == "players") {
                entry.players = result;
                entry.playersSrc = source;
            } else if (type == "ages") {
                entry.ages = result;
                entry.agesSrc = source;
            } else if (type == "tags") {
                entry.tags = result;
                entry.tagsSrc = source;
            } else if (type == "rating") {
                entry.rating = result;
                entry.ratingSrc = source;
            } else if (type == "releasedate") {
                entry.releaseDate = result;
                entry.releaseDateSrc = source;
            }
        }
    }

    for (auto const &type : binTypes()) {
        QString result = "";
        QString source = "";
        QByteArray data;
        if (fillType(type, matchingResources, result, source)) {
            QFile f(cacheDir.path() + "/" + result);
            if (f.open(QIODevice::ReadOnly)) {
                data = f.readAll();
                f.close();
            }
            if (type == "cover") {
                entry.coverData = data;
                entry.coverSrc = source;
            } else if (type == "screenshot") {
                entry.screenshotData = data;
                entry.screenshotSrc = source;
            } else if (type == "wheel") {
                entry.wheelData = data;
                entry.wheelSrc = source;
            } else if (type == "marquee") {
                entry.marqueeData = data;
                entry.marqueeSrc = source;
            } else if (type == "texture") {
                entry.textureData = data;
                entry.textureSrc = source;
            } else if (type == "video" && !data.isEmpty()) {
                // video is not part of artwork.xml / compositor.cpp
                // set filename here
                entry.videoData = data;
                entry.videoSrc = source;
                QFileInfo info(f);
                entry.videoFormat = info.suffix();
                entry.videoFile = info.absoluteFilePath();
            } else if (type == "manual" && !data.isEmpty()) {
                // manual is not part of artwork.xml / compositor.cpp
                // set filename here
                entry.manualData = data;
                entry.manualSrc = source;
                QFileInfo info(f);
                entry.manualFile = info.absoluteFilePath();
            }
        }
    }
}

bool Cache::fillType(const QString &type, QList<Resource> &matchingResources,
                     QString &result, QString &source) {
    QList<Resource> typeResources;
    for (const auto &resource : matchingResources) {
        if (resource.type == type) {
            typeResources.append(resource);
        }
    }
    if (typeResources.isEmpty()) {
        return false;
    }
    if (prioMap.contains(type)) {
        for (int a = 0; a < prioMap.value(type).length(); ++a) {
            for (const auto &resource : typeResources) {
                if (resource.source == prioMap.value(type).at(a)) {
                    result = resource.value;
                    source = resource.source;
                    return true;
                }
            }
        }
    }
    qint64 newest = 0;
    for (const auto &resource : typeResources) {
        if (resource.timestamp >= newest) {
            newest = resource.timestamp;
            result = resource.value;
            source = resource.source;
        }
    }
    return true;
}

bool Cache::removeMediaFile(Resource &res, const char *msg) {
    if (binTypes().contains(res.type) &&
        !QFile::remove(cacheDir.path() + "/" + res.value)) {
        printf(msg, res.value.toStdString().c_str());
        printf(", skipping...\n");
        return false;
    }
    return true;
}
