/***************************************************************************
 *            skyscraper.cpp
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

#include "config.h"

#include <QDebug>
#include <QDirIterator>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QProcess>
#include <QSettings>
#include <QStringBuilder>
#include <QThread>
#include <QTimer>
#include <iostream>

#if QT_VERSION >= 0x050400
#include <QStorageInfo>
#endif

#include "attractmode.h"
#include "cli.h"
#include "emulationstation.h"
#include "esde.h"
#include "pegasus.h"
#include "settings.h"
#include "skyscraper.h"
#include "strtools.h"

// https://stackoverflow.com/a/323302
static inline unsigned long mix(unsigned long a, unsigned long b,
                                unsigned long c) {
    a = a - b;
    a = a - c;
    a = a ^ (c >> 13);
    b = b - c;
    b = b - a;
    b = b ^ (a << 8);
    c = c - a;
    c = c - b;
    c = c ^ (b >> 13);
    a = a - b;
    a = a - c;
    a = a ^ (c >> 12);
    b = b - c;
    b = b - a;
    b = b ^ (a << 16);
    c = c - a;
    c = c - b;
    c = c ^ (b >> 5);
    a = a - b;
    a = a - c;
    a = a ^ (c >> 3);
    b = b - c;
    b = b - a;
    b = b ^ (a << 10);
    c = c - a;
    c = c - b;
    c = c ^ (b >> 15);
    return c;
}

Skyscraper::Skyscraper(const QString &currentDir) {
    qRegisterMetaType<GameEntry>("GameEntry");

    manager = QSharedPointer<NetManager>(new NetManager());

    // Randomize timer
#if QT_VERSION < 0x050a00
    qsrand(clock(), time(NULL), QCoreApplication::applicationPid());
#endif

    printf("%s", StrTools::getVersionHeader().toStdString().c_str());

    config.currentDir = currentDir;
}

Skyscraper::~Skyscraper() { frontend->deleteLater(); }

void Skyscraper::run() {

    if (config.platform.isEmpty()) {
        if (config.cacheOptions == "purge:all") {
            Cache::purgeAllPlatform(config, this);
            exit(0);
        } else if (config.cacheOptions.contains("report:missing")) {
            Cache::reportAllPlatform(config, this);
            exit(0);
        } else if (config.cacheOptions == "vacuum") {
            Cache::vacuumAllPlatform(config, this);
            exit(0);
        } else if (config.cacheOptions == "validate") {
            Cache::validateAllPlatform(config, this);
            exit(0);
        } else {
            exit(1);
        }
    }

    cacheScrapeMode = config.scraper == "cache";
    doCacheScraping = cacheScrapeMode && !config.pretend;
    printf("Platform:           '\033[1;32m%s\033[0m'\n",
           config.platform.toStdString().c_str());
    printf("Scraping module:    '\033[1;32m%s\033[0m'\n",
           config.scraper.toStdString().c_str());
    if (cacheScrapeMode) {
        printf("Frontend:           '\033[1;32m%s\033[0m'\n",
               config.frontend.toStdString().c_str());
        if (!config.frontendExtra.isEmpty()) {
            printf("Extra:              '\033[1;32m%s\033[0m'\n",
                   config.frontendExtra.toStdString().c_str());
        }
    }
    printf("Input folder:       '\033[1;32m%s\033[0m'\n",
           config.inputFolder.toStdString().c_str());
    printf("Game list folder:   '\033[1;32m%s\033[0m'\n",
           config.gameListFolder.toStdString().c_str());
    printf("Covers folder:      '\033[1;32m%s\033[0m'\n",
           config.coversFolder.toStdString().c_str());
    printf("Screenshots folder: '\033[1;32m%s\033[0m'\n",
           config.screenshotsFolder.toStdString().c_str());
    printf("Wheels folder:      '\033[1;32m%s\033[0m'\n",
           config.wheelsFolder.toStdString().c_str());
    printf("Marquees folder:    '\033[1;32m%s\033[0m'\n",
           config.marqueesFolder.toStdString().c_str());
    printf("Textures folder:    '\033[1;32m%s\033[0m'\n",
           config.texturesFolder.toStdString().c_str());
    if (config.videos) {
        printf("Videos folder:      '\033[1;32m%s\033[0m'\n",
               config.videosFolder.toStdString().c_str());
    }
    if (config.manuals) {
        printf("Manuals folder:     '\033[1;32m%s\033[0m'\n",
               config.manualsFolder.toStdString().c_str());
    }
    printf("Cache folder:       '\033[1;32m%s\033[0m'\n",
           config.cacheFolder.toStdString().c_str());
    if (config.scraper == "import") {
        printf("Import folder:      '\033[1;32m%s\033[0m'\n",
               config.importFolder.toStdString().c_str());
    }

    printf("\n");

    if (config.hints) {
        showHint();
    }

    prepareScraping();

    doneThreads = 0;
    notFound = 0;
    found = 0;
    avgCompleteness = 0;
    avgSearchMatch = 0;

    if (config.unpack) {
        QProcess decProc;
        decProc.setReadChannel(QProcess::StandardOutput);
        decProc.start("which", QStringList({"7z"}));
        decProc.waitForFinished(10000);
        if (!decProc.readAllStandardOutput().contains("7z")) {
            printf("Couldn't find '7z' command. 7z is required by the "
                   "'--flags unpack' flag. On Debian derivatives such as "
                   "RetroPie you can install it with 'sudo apt install "
                   "p7zip-full'.\n\nNow quitting...\n");
            exit(1);
        }
    }

    if (!config.cacheFolder.isEmpty()) {
        cache = QSharedPointer<Cache>(new Cache(config.cacheFolder));
        if (cacheScrapeMode || cache->createFolders(config.scraper)) {
            if (!cache->read() && cacheScrapeMode) {
                printf("No resources for this platform found in the resource "
                       "cache. Please specify a scraping module with '-s' to "
                       "gather some resources before trying to generate a game "
                       "list. Check all available modules with '--help'.\n");
                exit(1);
            }
        } else {
            printf("Couldn't create cache folders, please check folder "
                   "permissions and try again...\n");
            exit(1);
        }
    }

    if (config.verbosity || config.cacheOptions == "show") {
        cache->showStats(config.cacheOptions == "show" ? 2 : config.verbosity);
        if (config.cacheOptions == "show") {
            exit(0);
        }
    }
    if (config.cacheOptions.contains("purge:") ||
        config.cacheOptions.contains("vacuum")) {
        bool success = true;
        if (config.cacheOptions == "purge:all") {
            success = cache->purgeAll(config.unattend || config.unattendSkip);
        } else if (config.cacheOptions == "vacuum") {
            success = cache->vacuumResources(
                config.inputFolder, getPlatformFileExtensions(),
                config.verbosity, config.unattend || config.unattendSkip);
        } else if (config.cacheOptions.contains("purge:m=") ||
                   config.cacheOptions.contains("purge:t=")) {
            success = cache->purgeResources(config.cacheOptions);
        }
        if (success) {
            state = NO_INTR; // Ignore ctrl+c
            cache->write();
            state = SINGLE;
        }
        exit(0);
    }
    if (config.cacheOptions.contains("report:")) {
        cache->assembleReport(config, getPlatformFileExtensions());
        exit(0);
    }
    if (config.cacheOptions == "validate") {
        cache->validate();
        state = NO_INTR; // Ignore ctrl+c
        cache->write();
        state = SINGLE;
        exit(0);
    }
    if (config.cacheOptions.contains("merge:")) {
        QFileInfo mergeCacheInfo(config.cacheOptions.replace("merge:", ""));

        if (mergeCacheInfo.isRelative()) {
            mergeCacheInfo =
                QFileInfo(config.currentDir + "/" + mergeCacheInfo.filePath());
        }

        const QString absMergeCacheFilePath = mergeCacheInfo.absoluteFilePath();
        if (mergeCacheInfo.isDir()) {
            Cache mergeCache(absMergeCacheFilePath);
            mergeCache.read();
            cache->merge(mergeCache, config.refresh, absMergeCacheFilePath);
            state = NO_INTR; // Ignore ctrl+c
            cache->write();
            state = SINGLE;
        } else {
            printf("Path to merge from '%s' does not exist or is not a path, "
                   "can't continue...\n",
                   absMergeCacheFilePath.toStdString().c_str());
        }
        exit(0);
    }

    // remaining cache subcommand validation
    bool cacheEditCmd = config.cacheOptions.left(4) == "edit";
    if (!config.cacheOptions.isEmpty() && !cacheEditCmd) {
        printf("\033[1;31mAmbiguous cache subcommand '--cache %s', "
               "please check '--cache help' for more info.\n\033[0m",
               config.cacheOptions.toStdString().c_str());
        exit(0);
    }

    cache->readPriorities();

    // Create shared queue with files to process
    prepareFileQueue();

    state = CACHE_EDIT; // Clear queue on ctrl+c
    if (cacheEditCmd) {
        QString editCommand = "";
        QString editType = "";
        if (config.cacheOptions.contains(":") &&
            config.cacheOptions.contains("=")) {
            config.cacheOptions.remove(0, config.cacheOptions.indexOf(":") + 1);
            QStringList cacheOpts = config.cacheOptions.split("=");
            if (cacheOpts.size() == 2) {
                editCommand = cacheOpts.at(0);
                editType = cacheOpts.at(1);
            }
        }
        cache->editResources(queue, editCommand, editType);
        if (state == CACHE_EDIT) {
            printf("Done editing resources.\n");
            state = NO_INTR; // Ignore ctrl+c
            cache->write();
        } else {
            printf("Catched Ctrl-C: No changes persisted!\n");
        }
        state = SINGLE;
        exit(0);
    }
    state = SINGLE;

    gameListFileString =
        config.gameListFolder + "/" + frontend->getGameListFileName();

    QFile gameListFile(gameListFileString);

    if (doCacheScraping && config.gameListBackup) {
        QString gameListBackup =
            gameListFile.fileName() + "-" +
            QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        printf("Game list backup saved to '\033[1;33m%s\033[0m'\n",
               gameListBackup.toStdString().c_str());
        gameListFile.copy(gameListBackup);
    }

    if (doCacheScraping && !config.unattend && !config.unattendSkip &&
        gameListFile.exists()) {
        std::string userInput = "";
        printf("\033[1;34m'\033[0m\033[1;33m%s\033[0m\033[1;34m' already "
               "exists, do you want to overwrite it\033[0m (y/N)? ",
               frontend->getGameListFileName().toStdString().c_str());
        getline(std::cin, userInput);
        if (userInput != "y" && userInput != "Y") {
            printf("User chose not to overwrite, now exiting...\n");
            exit(0);
        }
        printf("Checking if '\033[1;33m%s\033[0m' is writable?... ",
               frontend->getGameListFileName().toStdString().c_str());

        if (gameListFile.open(QIODevice::Append)) {
            printf("\033[1;32mIt is! :)\033[0m\n");
            gameListFile.close();
        } else {
            printf("\033[1;31mIt isn't! :(\nPlease check path and permissions "
                   "and try again.\033[0m\n");
            exit(1);
        }
        printf("\n");
    }
    if (config.pretend && cacheScrapeMode) {
        printf("Pretend set! Not changing any files, just showing output.\n\n");
    }

    QFile::remove(skippedFileString);

    if (gameListFile.exists()) {
        printf("Trying to parse and load existing game list metadata... ");
        fflush(stdout);
        if (frontend->loadOldGameList(gameListFileString)) {
            printf("\033[1;32mSuccess!\033[0m\n");
            if (!config.unattend && cliFiles.isEmpty()) {
                std::string userInput = "";
                if (gameListFile.exists() && frontend->canSkip()) {
                    if (config.unattendSkip) {
                        userInput = "y";
                    } else {
                        printf("\033[1;34mDo you want to skip already existing "
                               "game list entries\033[0m (y/N)? ");
                        getline(std::cin, userInput);
                    }
                    if ((userInput == "y" || userInput == "Y") &&
                        frontend->canSkip()) {
                        frontend->skipExisting(gameEntries, queue);
                    }
                }
            }
        } else {
            printf("\033[1;33mNot found or unsupported!\033[0m\n");
        }
    }

    totalFiles = queue->length();
    if (totalFiles == 0) {
        QString extraInfo = doCacheScraping ? "in cache "
                                            : "matching these extensions " +
                                                  getPlatformFileExtensions();
        printf("\nNo files to process %s for platform "
               "'%s'.\nCheck configured and existing file extensions and cache "
               "content.\n\n\033[1;33mSkyscraper came to an untimely "
               "end.\033[0m\n\n",
               extraInfo.toStdString().c_str(),
               config.platform.toStdString().c_str());
        exit(0);
    }

    if (config.romLimit != -1 && totalFiles > config.romLimit) {
        int inCache = 0;
        if (config.onlyMissing) {
            // check queue on existing in cache and count
            for (int b = 0; b < queue->length(); ++b) {
                QFileInfo info = queue->at(b);
                QString cacheId = cache->getQuickId(info);
                if (!cacheId.isEmpty() && cache->hasEntries(cacheId)) {
                    // in cache from any scraping source
                    inCache++;
                }
            }
            qDebug() << "Only missing applied. Found" << inCache
                     << "existing game entries";
        }
        if (totalFiles - inCache > config.romLimit) {
            printf(
                "\n\033[1;33mRestriction overrun!\033[0m This scraping module "
                "only allows for scraping up to %d roms at a time. You can "
                "either supply a few rom filenames on command line, apply the "
                "--flags onlymissing option, or make use of the '--startat' "
                "and / or '--endat' command line options to adhere to this. "
                "Please check '--help' for more info.\n\nNow quitting...\n",
                config.romLimit);
            exit(0);
        }
    }
    printf("\n");
    if (!doCacheScraping) {
        printf("Starting scraping run on \033[1;32m%d\033[0m files using "
               "\033[1;32m%d\033[0m threads.\nSit back, relax and let me do "
               "the work! :)\n",
               totalFiles, config.threads);
    }
    printf("\n");

    timer.start();
    currentFile = 1;

    QList<QThread *> threadList;
    for (int curThread = 1; curThread <= config.threads; ++curThread) {
        QThread *thread = new QThread;
        ScraperWorker *worker = new ScraperWorker(queue, cache, manager, config,
                                                  QString::number(curThread));
        worker->moveToThread(thread);
        connect(thread, &QThread::started, worker, &ScraperWorker::run);
        connect(worker, &ScraperWorker::entryReady, this,
                &Skyscraper::entryReady);
        connect(worker, &ScraperWorker::allDone, this,
                &Skyscraper::checkThreads);
        connect(thread, &QThread::finished, worker,
                &ScraperWorker::deleteLater);
        threadList.append(thread);
        // Do not start more threads if we have less files than allowed threads
        if (curThread == totalFiles) {
            config.threads = curThread;
            break;
        }
    }
    // Ready, set, GO! Start all threads
    for (const auto thread : threadList) {
        thread->start();
        state = THREADED;
    }
}

void Skyscraper::prepareFileQueue() {
    QDir::Filters filter = QDir::Files;
    // special case scummvm: users can use .svm in folder name to work around
    // the limitation of the ScummVM / lr-scummvm launch integration in
    // ES/RetroPie
    if (config.platform == "scummvm") {
        filter |= QDir::Dirs;
    }
    QDir inputDir(config.inputFolder, getPlatformFileExtensions(), QDir::Name,
                  filter);
    if (!inputDir.exists()) {
        printf("Input folder '\033[1;32m%s\033[0m' doesn't exist or can't be "
               "accessed by current user. Please check path and permissions.\n",
               inputDir.absolutePath().toStdString().c_str());
        exit(1);
    }
    config.inputFolder = inputDir.absolutePath();

    setFolder(doCacheScraping, config.gameListFolder);
    setFolder(doCacheScraping, config.coversFolder);
    setFolder(doCacheScraping, config.screenshotsFolder);
    setFolder(doCacheScraping, config.wheelsFolder);
    setFolder(doCacheScraping, config.marqueesFolder);
    setFolder(doCacheScraping, config.texturesFolder);
    if (config.videos) {
        setFolder(doCacheScraping, config.videosFolder);
    }
    if (config.manuals) {
        setFolder(doCacheScraping, config.manualsFolder);
    }

    setFolder(doCacheScraping, config.importFolder, false);

    QList<QFileInfo> infoList = inputDir.entryInfoList();
    if (!cacheScrapeMode &&
        QFileInfo::exists(config.inputFolder + "/.skyscraperignore")) {
        infoList.clear();
    }
    if (!config.startAt.isEmpty() && !infoList.isEmpty()) {
        QFileInfo startAt(config.startAt);
        if (!startAt.exists()) {
            startAt.setFile(config.currentDir + "/" + config.startAt);
        }
        if (!startAt.exists()) {
            startAt.setFile(inputDir.absolutePath() + "/" + config.startAt);
        }
        if (startAt.exists()) {
            while (infoList.first().fileName() != startAt.fileName() &&
                   !infoList.isEmpty()) {
                infoList.removeFirst();
            }
        }
    }
    if (!config.endAt.isEmpty() && !infoList.isEmpty()) {
        QFileInfo endAt(config.endAt);
        if (!endAt.exists()) {
            endAt.setFile(config.currentDir + "/" + config.endAt);
        }
        if (!endAt.exists()) {
            endAt.setFile(inputDir.absolutePath() + "/" + config.endAt);
        }
        if (endAt.exists()) {
            while (infoList.last().fileName() != endAt.fileName() &&
                   !infoList.isEmpty()) {
                infoList.removeLast();
            }
        }
    }

    queue = QSharedPointer<Queue>(new Queue());
    queue->append(infoList);

    if (config.subdirs) {
        QDirIterator dirIt(config.inputFolder,
                           QDir::Dirs | QDir::NoDotAndDotDot,
                           QDirIterator::Subdirectories);
        QString exclude = "";
        while (dirIt.hasNext()) {
            QString subdir = dirIt.next();
            if (!cacheScrapeMode &&
                QFileInfo::exists(subdir + "/.skyscraperignoretree")) {
                exclude = subdir;
            }
            if (!exclude.isEmpty() &&
                (subdir == exclude ||
                 (subdir.left(exclude.length()) == exclude &&
                  subdir.mid(exclude.length(), 1) == "/"))) {
                continue;
            } else {
                exclude.clear();
            }
            if (!cacheScrapeMode &&
                QFileInfo::exists(subdir + "/.skyscraperignore")) {
                continue;
            }
            inputDir.setPath(subdir);
            queue->append(inputDir.entryInfoList());
            if (config.verbosity > 0) {
                printf("Adding files from subdir: '%s'\n",
                       subdir.toStdString().c_str());
            }
        }
        if (config.verbosity > 0)
            printf("\n");
    }
    if (!config.excludePattern.isEmpty()) {
        queue->filterFiles(config.excludePattern);
    }
    if (!config.includePattern.isEmpty()) {
        queue->filterFiles(config.includePattern, true);
    }

    if (!cliFiles.isEmpty()) {
        queue->clear();
        for (const auto &cliFile : cliFiles) {
            queue->append(QFileInfo(cliFile));
        }
    }

    // Remove files from excludeFrom, if any
    if (!config.excludeFrom.isEmpty()) {
        queue->removeFiles(readFileListFrom(config.excludeFrom));
    }
}

void Skyscraper::setFolder(const bool doCacheScraping, QString &outFolder,
                           const bool createMissingFolder) {
    QDir dir(outFolder);
    if (doCacheScraping) {
        checkForFolder(dir, createMissingFolder);
    }
    outFolder = dir.absolutePath();
}

void Skyscraper::checkForFolder(QDir &folder, bool create) {
    if (!folder.exists()) {
        printf("Folder '%s' doesn't exist",
               folder.absolutePath().toStdString().c_str());
        if (create) {
            printf(", trying to create it... ");
            fflush(stdout);
            if (folder.mkpath(folder.absolutePath())) {
                printf("\033[1;32mSuccess!\033[0m\n");
            } else {
                printf("\033[1;32mFailed!\033[0m Please check path and "
                       "permissions, now exiting...\n");
                exit(1);
            }
        } else {
            printf(", can't continue...\n");
            exit(1);
        }
    }
}

QString Skyscraper::secsToString(const int &secs) {
    QString hours = QString::number(secs / 3600000 % 24);
    QString minutes = QString::number(secs / 60000 % 60);
    QString seconds = QString::number(secs / 1000 % 60);
    if (hours.length() == 1) {
        hours.prepend("0");
    }
    if (minutes.length() == 1) {
        minutes.prepend("0");
    }
    if (seconds.length() == 1) {
        seconds.prepend("0");
    }

    return hours + ":" + minutes + ":" + seconds;
}

void Skyscraper::entryReady(const GameEntry &entry, const QString &output,
                            const QString &debug) {
    QMutexLocker locker(&entryMutex);

    printf("\033[0;32m#%d/%d\033[0m %s\n", currentFile, totalFiles,
           output.toStdString().c_str());

    if (config.verbosity >= 3) {
        printf("\033[1;33mDebug output:\033[0m\n%s\n",
               debug.toStdString().c_str());
    }

    if (entry.found) {
        found++;
        avgCompleteness += entry.getCompleteness();
        avgSearchMatch += entry.searchMatch;
        gameEntries.append(entry);
    } else {
        notFound++;
        QFile skippedFile(skippedFileString);
        skippedFile.open(QIODevice::Append);
        skippedFile.write(entry.absoluteFilePath.toUtf8() + "\n");
        skippedFile.close();
        if (config.skipped) {
            gameEntries.append(entry);
        }
    }

    printf(
        "\033[1;34m#%d/%d\033[0m, (\033[1;32m%d\033[0m/\033[1;33m%d\033[0m)\n",
        currentFile, totalFiles, found, notFound);
    int elapsed = timer.elapsed();
    int estTime = (elapsed / currentFile * totalFiles) - elapsed;
    if (estTime < 0)
        estTime = 0;
    printf("Elapsed time   : \033[1;33m%s\033[0m\n",
           secsToString(elapsed).toStdString().c_str());
    printf("Est. time left : \033[1;33m%s\033[0m\n\n",
           secsToString(estTime).toStdString().c_str());

    if (!config.onlyMissing && currentFile == config.maxFails &&
        notFound == config.maxFails && config.scraper != "import" &&
        config.scraper != "cache") {
        printf("\033[1;31mThis is NOT going well! I guit! *slams the "
               "door*\nNo, seriously, out of %d files we had %d misses. So "
               "either the scraping source is down or you are using a scraping "
               "source that doesn't support this platform. Please try another "
               "scraping module (check '--help').\n\nNow exiting...\033[0m\n",
               config.maxFails, config.maxFails);
        exit(1);
    }
    currentFile++;

#if QT_VERSION >= 0x050400
    qint64 spaceLimit = 209715200;
    if (config.spaceCheck) {
        if (config.scraper == "cache" && !config.pretend &&
            QStorageInfo(QDir(config.screenshotsFolder)).bytesFree() <
                spaceLimit) {
            printf("\033[1;31mYou have very little disk space left on the "
                   "Skyscraper media export drive, please free up some space "
                   "and try again. Now aborting...\033[0m\n\n");
            printf("Note! You can disable this check by setting "
                   "'spaceCheck=\"false\"' in the '[main]' section of "
                   "config.ini.\n\n");
            // By clearing the queue here we basically tell Skyscraper to stop
            // and quit nicely
            config.pretend = true;
            queue->clearAll();
        } else if (QStorageInfo(QDir(config.cacheFolder)).bytesFree() <
                   spaceLimit) {
            printf("\033[1;31mYou have very little disk space left on the "
                   "Skyscraper resource cache drive, please free up some space "
                   "and try again. Now aborting...\033[0m\n\n");
            printf("Note! You can disable this check by setting "
                   "'spaceCheck=\"false\"' in the '[main]' section of "
                   "config.ini.\n\n");
            // By clearing the queue here we basically tell Skyscraper to stop
            // and quit nicely
            config.pretend = true;
            queue->clearAll();
        }
    }
#endif
}

void Skyscraper::checkThreads() {
    QMutexLocker locker(&checkThreadMutex);

    doneThreads++;
    if (doneThreads != config.threads)
        return;

    if (!config.pretend && config.scraper == "cache") {
        printf("\033[1;34m---- Game list generation run completed! YAY! "
               "----\033[0m\n");
        if (!config.cacheFolder.isEmpty()) {
            state = NO_INTR; // Ignore ctrl+c
            cache->write(true);
            state = SINGLE;
        }
        frontend->sortEntries(gameEntries);
        printf("Assembling game list...");
        QString finalOutput;
        frontend->assembleList(finalOutput, gameEntries);
        printf(" \033[1;32mDone!\033[0m\n");
        QFile gameListFile(gameListFileString);
        printf("Now writing '\033[1;33m%s\033[0m'... ",
               gameListFileString.toStdString().c_str());
        fflush(stdout);
        if (gameListFile.open(QIODevice::WriteOnly)) {
            state = NO_INTR; // Ignore ctrl+c
            gameListFile.write(finalOutput.toUtf8());
            state = SINGLE;
            gameListFile.close();
            printf("\033[1;32mSuccess!\033[0m\n\n");
        } else {
            printf("\033[1;31mCouldn't open file for writing!\nAll that work "
                   "for nothing... :(\033[0m\n");
        }
    } else {
        printf("\033[1;34m---- Resource gathering run completed! YAY! "
               "----\033[0m\n");
        if (!config.cacheFolder.isEmpty()) {
            state = NO_INTR; // Ignore ctrl+c
            cache->write();
            state = SINGLE;
        }
    }

    if (!doCacheScraping || totalFiles > 0) {
        printf("\033[1;34m---- And here are some neat stats :) ----\033[0m\n");
    }
    if (!doCacheScraping) {
        printf("Total completion time: \033[1;33m%s\033[0m\n\n",
               secsToString(timer.elapsed()).toStdString().c_str());
    }
    if (totalFiles > 0) {
        if (found > 0) {
            printf("Average search match: \033[1;33m%d%%\033[0m\n",
                   (int)((double)avgSearchMatch / (double)found));
            printf("Average entry completeness: \033[1;33m%d%%\033[0m\n\n",
                   (int)((double)avgCompleteness / (double)found));
        }
        printf("\033[1;34mTotal number of games: %d\033[0m\n", totalFiles);
        printf("\033[1;32mSuccessfully processed games: %d\033[0m\n", found);
        printf("\033[1;33mSkipped games: %d\033[0m", notFound);
        if (notFound > 0) {
            printf(" (Filenames saved to '\033[1;33m%s/%s\033[0m')",
                   Config::getSkyFolder(Config::SkyFolderType::LOG)
                       .toStdString()
                       .c_str(),
                   skippedFileString.toStdString().c_str());
        }
        printf("\n\n");
    }

    // All done, now clean up and exit to terminal
    emit finished();
}

QList<QString> Skyscraper::readFileListFrom(const QString &filename) {
    QList<QString> fileList;
    QFileInfo fnInfo(filename);
    if (!fnInfo.exists()) {
        fnInfo.setFile(config.currentDir + "/" + filename);
    }
    if (fnInfo.exists()) {
        QFile f(fnInfo.absoluteFilePath());
        if (f.open(QIODevice::ReadOnly)) {
            while (!f.atEnd()) {
                fileList.append(QString(f.readLine().simplified()));
            }
            f.close();
        }
    } else {
        printf("File: '\033[1;32m%s\033[0m' does not exist.\n\nPlease "
               "verify the filename and try again...\n",
               fnInfo.absoluteFilePath().toStdString().c_str());
        exit(1);
    }
    return fileList;
}

void Skyscraper::loadConfig(const QCommandLineParser &parser) {
    QString iniFile = parser.isSet("c") ? parser.value("c") : "config.ini";
    if (!QFileInfo(iniFile).exists()) {
        printf(
            "\nWARNING! Provided config file '\033[1;33m%s\033[0m' does not "
            "exist.\nSkyscraper will use default configuration values...\n\n",
            iniFile.toStdString().c_str());
    }
    QSettings settings(iniFile, QSettings::IniFormat);

    RuntimeCfg *rtConf = new RuntimeCfg(&config, &parser);
    // Start by setting frontend, since we need it to set default for game list
    // and so on
    if (parser.isSet("f")) {
        QString fe = parser.value("f");
        if (!rtConf->validateFrontend(fe)) {
            exit(1);
        }
        config.frontend = fe;
    }

    bool inputFolderSet = false;
    bool gameListFolderSet = false;
    bool mediaFolderSet = false;

    // 1. Main config, overrides defaults
    settings.beginGroup("main");
    rtConf->applyConfigIni(RuntimeCfg::CfgType::MAIN, &settings, inputFolderSet,
                           gameListFolderSet, mediaFolderSet);
    settings.endGroup();

    // 2. Platform specific configs, overrides main and defaults
    settings.beginGroup(config.platform);
    rtConf->applyConfigIni(RuntimeCfg::CfgType::PLATFORM, &settings,
                           inputFolderSet, gameListFolderSet, mediaFolderSet);
    settings.endGroup();

    // Check for command line scraping module here
    QStringList scrapers = {"arcadedb",       "cache",         "esgamelist",
                            "igdb",           "import",        "mobygames",
                            "openretro",      "screenscraper", "thegamesdb",
                            "worldofspectrum"};
    if (parser.isSet("s")) {
        if (scrapers.contains(parser.value("s"))) {
            config.scraper = parser.value("s");
        } else {
            printf("\033[1;31mBummer! Unknown scrapingmodule '%s'. Known "
                   "scrapers are: %s.\nHint: Try TAB-completion to avoid "
                   "typos.\033[0m\n",
                   parser.value("s").toStdString().c_str(),
                   scrapers.join(", ").toStdString().c_str());
            exit(1);
        }
    }

    // 3. Frontend specific configs, overrides platform, main and
    // defaults
    settings.beginGroup(config.frontend);
    rtConf->applyConfigIni(RuntimeCfg::CfgType::FRONTEND, &settings,
                           inputFolderSet, gameListFolderSet, mediaFolderSet);
    settings.endGroup();

    // 4. Scraping module specific configs, overrides frontend, platform,
    // main and defaults
    settings.beginGroup(config.scraper);
    rtConf->applyConfigIni(RuntimeCfg::CfgType::SCRAPER, &settings,
                           inputFolderSet, gameListFolderSet, mediaFolderSet);
    settings.endGroup();

    // 5. Command line configs, overrides all
    rtConf->applyCli(inputFolderSet, gameListFolderSet, mediaFolderSet);

    if (config.platform.isEmpty() && !config.cacheOptions.isEmpty()) {
        return; // cache option to be applied to all platform
    }

    if (config.frontend == "emulationstation" ||
        config.frontend == "retrobat") {
        frontend = new EmulationStation;
    } else if (config.frontend == "attractmode") {
        frontend = new AttractMode;
    } else if (config.frontend == "pegasus") {
        frontend = new Pegasus;
    } else if (config.frontend == "esde") {
        frontend = new Esde;
    }

    frontend->setConfig(&config);
    frontend->checkReqs();

    // Change config defaults if they aren't already set, find the rest in
    // settings.h
    if (!inputFolderSet) {
        config.inputFolder = frontend->getInputFolder();
    }
    if (!gameListFolderSet) {
        config.gameListFolder = frontend->getGameListFolder();
    }
    if (!mediaFolderSet) {
        if (config.frontend == "esde") {
            config.mediaFolder = frontend->getMediaFolder();
        } else {
            // defaults to <gamelistfolder>/[.]media/
            QString mf = "media";
            if (config.mediaFolderHidden) {
                mf = "." + mf;
            }
            config.mediaFolder = rtConf->concatPath(config.gameListFolder, mf);
        }
    }
    // only resolve after config.mediaFolder is set
    config.coversFolder = frontend->getCoversFolder();
    config.screenshotsFolder = frontend->getScreenshotsFolder();
    config.wheelsFolder = frontend->getWheelsFolder();
    config.marqueesFolder = frontend->getMarqueesFolder();
    config.texturesFolder = frontend->getTexturesFolder();
    config.videosFolder = frontend->getVideosFolder();
    config.manualsFolder = frontend->getManualsFolder();

    // Choose default scraper for chosen platform if none has been set yet
    if (config.scraper.isEmpty()) {
        config.scraper = "cache";
    }

    // If platform subfolder exists for import path, use it
    QDir importFolder(config.importFolder);
    if (importFolder.exists(config.platform)) {
        config.importFolder =
            rtConf->concatPath(config.importFolder, config.platform);
    }

    // Set minMatch to 0 for cache, arcadedb and screenscraper
    // We know these results are always accurate
    if (config.minMatchSet == false && config.isMatchOneScraper()) {
        config.minMatch = 0;
    }

    skippedFileString =
        "skipped-" + config.platform + "-" + config.scraper + ".txt";

    // Grab all requested files from cli, if any
    QList<QString> requestedFiles = parser.positionalArguments();

    // Add files from '--includefrom', if any
    if (!config.includeFrom.isEmpty()) {
        requestedFiles += readFileListFrom(config.includeFrom);
    }

    // Verify requested files and add the ones that exist
    for (const auto &requestedFile : requestedFiles) {
        QFileInfo requestedFileInfo(requestedFile);
        if (!requestedFileInfo.exists()) {
            requestedFileInfo.setFile(config.currentDir + "/" + requestedFile);
        }
        if (!requestedFileInfo.exists()) {
            requestedFileInfo.setFile(config.inputFolder + "/" + requestedFile);
        }
        if (requestedFileInfo.exists()) {
            QString romPath = requestedFileInfo.absoluteFilePath();
            if (config.frontend == "emulationstation" ||
                config.frontend == "esde") {
                romPath = normalizePath(requestedFileInfo);
            }
            if (!romPath.isEmpty()) {
                cliFiles.append(romPath);
                // Always set refresh and unattend true if user has supplied
                // filenames on command line. That way they are cached, but game
                // list is not changed and user isn't asked about skipping and
                // overwriting.
                config.refresh = true;
                config.unattend = true;
                continue;
            }
        }
        printf("Filename: '\033[1;32m%s\033[0m' requested either on "
               "command line or with '--includefrom' not found!\n\nPlease "
               "verify the filename and try again...\n",
               requestedFile.toStdString().c_str());
        exit(1);
    }

    // Add query only if a single filename was passed on command line
    if (parser.isSet("query")) {
        if (cliFiles.length() == 1) {
            config.searchName = parser.value("query");
        } else {
            printf("'--query' requires a single rom filename to be added at "
                   "the end of the command-line. You either forgot to set one, "
                   "or more than one was provided. Now quitting...\n");
            exit(1);
        }
    }

    if (config.startAt != "" || config.endAt != "") {
        config.refresh = true;
        config.unattend = true;
        config.subdirs = false;
    }

    // If interactive is set, force 1 thread and always accept the chosen result
    // but only on selected scrape modules
    if (config.interactive) {
        if (config.isMatchOneScraper()) {
            config.interactive = false;
        } else {
            config.threads = 1;
            config.minMatch = 0;
            config.refresh = true;
        }
    }

    if (config.scraper == "import") {
        // Always force the cache to be refreshed when using import scraper
        config.refresh = true;
        config.videos = true;
        // minMatch set to 0 further up
    }

    if (!config.userCreds.isEmpty() && config.userCreds.contains(":")) {
        QList<QString> userCreds = config.userCreds.split(":");
        if (userCreds.length() == 2) {
            config.user = userCreds.at(0);
            config.password = userCreds.at(1);
        }
    }

    QFile artworkFile(config.artworkConfig);
    if (artworkFile.open(QIODevice::ReadOnly)) {
        config.artworkXml = artworkFile.readAll();
        artworkFile.close();
    } else {
        printf("Couldn't read artwork xml file '\033[1;32m%s\033[0m'. Please "
               "check file and permissions. Now exiting...\n",
               config.artworkConfig.toStdString().c_str());
        exit(1);
    }

    QDir resDir(Config::getSkyFolder(Config::SkyFolderType::RESOURCE));
    QDirIterator resDirIt(resDir.absolutePath(),
                          QDir::Files | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories);
    const QString resFolder = "resources/";
    while (resDirIt.hasNext()) {
        QString resFile = resDirIt.next();
        // reduce key to relative filepath
        resFile =
            resFile.remove(0, resFile.indexOf(resFolder) + resFolder.length());
        config.resources[resFile] = QImage(resFolder % resFile);
    }
}

QString Skyscraper::normalizePath(QFileInfo fileInfo) {
    // normalize paths for single romfiles provided at the CLI.
    // format will be: config.inputFolder + relative-path-of-romfile
    QString canonicalRomPath = fileInfo.canonicalFilePath();
    // for Windows
    QString cleanRomPath = QDir::cleanPath(canonicalRomPath);
    QListIterator<QString> iter(cleanRomPath.split("/"));
    iter.toBack();
    QString relativeRomPath;
    while (iter.hasPrevious()) {
        relativeRomPath = iter.previous() %
                          (relativeRomPath.isEmpty() ? "" : "/") %
                          relativeRomPath;
        QFileInfo normRom(config.inputFolder % "/" % relativeRomPath);
        if (normRom.exists()) {
            qDebug() << "CLI romfilepath normalized to: "
                     << normRom.absoluteFilePath();
            return normRom.absoluteFilePath();
        }
    }
    return "";
}

void Skyscraper::showHint() { Cli::showHint(); }

void Skyscraper::prepareScraping() {
    loadAliasMap();
    loadMameMap();

    setRegionPrios();
    setLangPrios();

    NetComm netComm(manager);
    QEventLoop q; // Event loop for use when waiting for data from NetComm.
    connect(&netComm, &NetComm::dataReady, &q, &QEventLoop::quit);

    if (config.platform == "amiga" && config.scraper != "cache" &&
        config.scraper != "import" && config.scraper != "esgamelist") {
        updateWhdloadDb(netComm, q);
    }
    loadWhdLoadMap();

    if (config.scraper == "arcadedb" && config.threads != 1) {
        printf("\033[1;33mForcing 1 thread to accomodate limits in the "
               "ArcadeDB API\033[0m\n\n");
        // Don't change! This limit was set by request from ArcadeDB
        config.threads = 1;
    } else if (config.scraper == "openretro" && config.threads != 1) {
        printf("\033[1;33mForcing 1 thread to accomodate limits in the "
               "OpenRetro API\033[0m\n\n");
        // Don't change! This limit was set by request from OpenRetro
        config.threads = 1;
    } else if (config.scraper == "igdb") {
        prepareIgdb(netComm, q);
    } else if (config.scraper == "mobygames" && config.threads != 1) {
        printf(
            "\033[1;33mForcing 1 thread to accomodate limits in MobyGames "
            "scraping module. Also be aware that MobyGames has a request limit "
            "of 360 requests per hour for the entire Skyscraper user base. So "
            "if someone else is currently using it, it will quit.\033[0m\n\n");
        // Don't change these! This limit was set by request from Mobygames
        config.threads = 1;
        config.romLimit = 35;
    } else if (config.scraper == "screenscraper") {
        prepareScreenscraper(netComm, q);
    }
}

void Skyscraper::updateWhdloadDb(NetComm &netComm, QEventLoop &q) {
    const QString url = "https://raw.githubusercontent.com/HoraceAndTheSpider/"
                        "Amiberry-XML-Builder/master/whdload_db.xml";
    netComm.request(url, "HEAD");
    q.exec();

    QString etag = netComm.getHeaderValue("ETag");
    QString cachedEtag;
    QFile whdlEtagFile(Config::getSkyFolder(Config::SkyFolderType::LOG) %
                       "/whdload_cached_etag.txt");
    if (whdlEtagFile.open(QIODevice::ReadOnly)) {
        cachedEtag = QString(whdlEtagFile.readLine());
        whdlEtagFile.close();
    }
    if (!etag.isEmpty() && etag == cachedEtag) {
        // not expired, no need to download again
        return;
    }

    if (!etag.isEmpty() && whdlEtagFile.open(QIODevice::WriteOnly)) {
        whdlEtagFile.write(etag.toUtf8());
        whdlEtagFile.close();
    }

    printf("Fetching 'whdload_db.xml', just a sec...");
    netComm.request(url);
    q.exec();
    QByteArray data = netComm.getData();
    QDomDocument tempDoc;
    QFile whdLoadFile("whdload_db.xml");
    if (data.size() > 1000 * 1000 && tempDoc.setContent(data) &&
        whdLoadFile.open(QIODevice::WriteOnly)) {
        whdLoadFile.write(data);
        whdLoadFile.close();
        printf("\033[1;32m Success!\033[0m\n\n");
    } else {
        printf("\033[1;31m Failed!\033[0m\n\n");
    }
}

void Skyscraper::prepareIgdb(NetComm &netComm, QEventLoop &q) {
    if (config.threads > 4) {
        // Don't change! This limit was set by request from IGDB
        config.threads = 4;
        printf("\033[1;33mAdjusting to %d threads to accomodate limits in "
               "the IGDB API\033[0m\n\n",
               config.threads);
        printf("\033[1;32mTHIS MODULE IS POWERED BY IGDB.COM\033[0m\n");
    }
    if (config.user.isEmpty() || config.password.isEmpty()) {
        printf("The IGDB scraping module requires free user credentials to "
               "work. Read more about that here: "
               "'https://gemba.github.io/skyscraper/SCRAPINGMODULES#igdb'\n");
        exit(1);
    }
    printf("Fetching IGDB authentication token status, just a sec...\n");
    QFile tokenFile("igdbToken.dat");
    QByteArray tokenData = "";
    if (tokenFile.exists() && tokenFile.open(QIODevice::ReadOnly)) {
        tokenData = tokenFile.readAll().trimmed();
        tokenFile.close();
    }
    if (tokenData.split(';').length() != 3) {
        tokenData = "user;token;0";
    }
    bool updateToken = false;
    if (config.user != tokenData.split(';').at(0)) {
        updateToken = true;
    }
    qlonglong tokenLife = tokenData.split(';').at(2).toLongLong() -
                          (QDateTime::currentMSecsSinceEpoch() / 1000);
    // 2 days, should be plenty for a scraping run
    if (tokenLife < 60 * 60 * 24 * 2) {
        updateToken = true;
    }
    config.igdbToken = tokenData.split(';').at(1);
    if (updateToken) {
        netComm.request("https://id.twitch.tv/oauth2/token"
                        "?client_id=" +
                            config.user + "&client_secret=" + config.password +
                            "&grant_type=client_credentials",
                        "");
        q.exec();
        QJsonObject jsonObj =
            QJsonDocument::fromJson(netComm.getData()).object();
        if (jsonObj.contains("access_token") &&
            jsonObj.contains("expires_in") && jsonObj.contains("token_type")) {
            config.igdbToken = jsonObj["access_token"].toString();
            printf("Token '%s' acquired, ready to scrape!\n",
                   config.igdbToken.toStdString().c_str());
            tokenLife = (QDateTime::currentMSecsSinceEpoch() / 1000) +
                        jsonObj["expires_in"].toInt();
            if (tokenFile.open(QIODevice::WriteOnly)) {
                tokenFile.write(
                    config.user.toUtf8() + ";" + config.igdbToken.toUtf8() +
                    ";" +
                    QByteArray::number(
                        (QDateTime::currentMSecsSinceEpoch() / 1000) +
                        tokenLife));
                tokenFile.close();
            }
        } else {
            printf("\033[1;33mReceived invalid IGDB server response. This "
                   "can be caused by server issues or maybe you entered "
                   "your credentials incorrectly in the Skyscraper "
                   "configuration. Read more about that here: "
                   "'https://gemba.github.io/skyscraper/"
                   "SCRAPINGMODULES#igdb'\033[0m\n");
            exit(1);
        }
    } else {
        printf("Cached token '%s' still valid, ready to scrape!\n",
               config.igdbToken.toStdString().c_str());
    }
    printf("\n");
}

void Skyscraper::prepareScreenscraper(NetComm &netComm, QEventLoop &q) {
    const int threadsFailsafe = 1; // Don't change! This limit was set by
                                   // request from ScreenScraper
    if ((config.user.isEmpty() || config.password.isEmpty()) &&
        config.threads > 1) {
        config.threads = threadsFailsafe;
        printf("\033[1;33mForcing %d thread as this is the anonymous "
               "limit in the ScreenScraper scraping module. Sign up for "
               "an account at https://www.screenscraper.fr and support "
               "them to gain more threads. Then use the credentials with "
               "Skyscraper using the '-u user:password' command line "
               "option or by setting 'userCreds=\"user:password\"' in "
               "'%s/config.ini'.\033[0m\n\n",
               config.threads, Config::getSkyFolder().toStdString().c_str());
    } else {
        printf("Fetching limits for user '\033[1;33m%s\033[0m', just a "
               "sec...\n",
               config.user.toStdString().c_str());
        netComm.request(
            "https://www.screenscraper.fr/api2/"
            "ssuserInfos.php?devid=muldjord&devpassword=" +
            StrTools::unMagic("204;198;236;130;203;181;203;126;191;167;200;"
                              "198;192;228;169;156") +
            "&softname=skyscraper" VERSION "&output=json&ssid=" + config.user +
            "&sspassword=" + config.password);
        q.exec();
        QJsonObject jsonObj =
            QJsonDocument::fromJson(netComm.getData()).object();
        if (jsonObj.isEmpty()) {
            config.threads = threadsFailsafe;
            if (netComm.getData().contains("Erreur de login")) {
                printf("\033[0;31mScreenScraper login error! Please verify "
                       "that you've entered your credentials correctly in "
                       "'%s/config.ini'. It needs to "
                       "look EXACTLY like this, but with your USER and "
                       "PASS:\033[0m\n\033[1;33m[screenscraper]\nuserCreds="
                       "\"USER:PASS\"\033[0m\033[0;31m\nContinuing with "
                       "unregistered user, forcing %d thread...\033[0m\n\n",
                       Config::getSkyFolder().toStdString().c_str(),
                       config.threads);
            } else {
                printf("\033[1;33mReceived invalid / empty ScreenScraper "
                       "server response, maybe their server is busy / "
                       "overloaded. Forcing 1 thread...\033[0m\n\n");
            }
        } else {
            int allowedThreads = jsonObj["response"]
                                     .toObject()["ssuser"]
                                     .toObject()["maxthreads"]
                                     .toString()
                                     .toInt();
            if (allowedThreads != 0) {
                if (config.threadsSet && config.threads <= allowedThreads) {
                    printf("User is allowed %d threads, but user has set "
                           "it manually, so ignoring.\n\n",
                           allowedThreads);
                } else {
                    config.threads = (allowedThreads <= 8 ? allowedThreads : 8);
                    printf("Setting threads to \033[1;32m%d\033[0m as "
                           "allowed for the supplied user credentials.\n\n",
                           config.threads);
                }
            }
        }
    }
}

void Skyscraper::loadAliasMap() {
    QFile aliasMapFile("aliasMap.csv");
    if (aliasMapFile.open(QIODevice::ReadOnly)) {
        while (!aliasMapFile.atEnd()) {
            QByteArray line = aliasMapFile.readLine();
            if (line.left(1) == "#") {
                continue;
            }
            QList<QByteArray> pair = line.split(';');
            if (pair.size() == 2) {
                QString baseName = pair.at(0);
                QString aliasName = pair.at(1);
                baseName = baseName.replace("\"", "").simplified();
                aliasName = aliasName.replace("\"", "").simplified();
                config.aliasMap[baseName] = aliasName;
            }
        }
        aliasMapFile.close();
    }
}

void Skyscraper::loadMameMap() {
    QFile mameMapFile("mameMap.csv");
    if (config.arcadePlatform && mameMapFile.open(QIODevice::ReadOnly)) {
        while (!mameMapFile.atEnd()) {
            QList<QByteArray> pair = mameMapFile.readLine().split(';');
            if (pair.size() != 2)
                continue;
            QString mameName = pair.at(0);
            QString realName = pair.at(1);
            mameName = mameName.replace("\"", "").simplified();
            realName = realName.replace("\"", "").simplified();
            config.mameMap[mameName] = realName;
        }
        mameMapFile.close();
    }
}

void Skyscraper::loadWhdLoadMap() {
    if (config.platform != "amiga") {
        return;
    }

    QFile whdLoadFile;
    QStringList paths = {
        "whdload_db.xml",
        "/opt/retropie/emulators/amiberry/whdboot/game-data/whdload_db.xml"};
    bool found = false;
    for (const auto &p : paths) {
        if (QFileInfo::exists(p)) {
            whdLoadFile.setFileName(p);
            found = true;
            break;
        }
    }
    if (!found || !whdLoadFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QDomDocument doc;
    QByteArray rawXml = whdLoadFile.readAll();
    whdLoadFile.close();
    if (doc.setContent(rawXml)) {
        QDomNodeList gameNodes = doc.elementsByTagName("game");
        for (int a = 0; a < gameNodes.length(); ++a) {
            QDomNode gameNode = gameNodes.at(a);
            QPair<QString, QString> gamePair;
            gamePair.first = gameNode.firstChildElement("name").text();
            gamePair.second = gameNode.firstChildElement("variant_uuid").text();
            config.whdLoadMap[gameNode.toElement().attribute("filename")] =
                gamePair;
        }
    }
}

void Skyscraper::setRegionPrios() {
    // Load single custom region
    if (!config.region.isEmpty()) {
        config.regionPrios.append(config.region);
    }

    // Load custom region prioritizations
    if (!config.regionPriosStr.isEmpty()) {
        for (const auto &region : config.regionPriosStr.split(",")) {
            QString r = region.trimmed();
            if (!config.regionPrios.contains(r)) {
                config.regionPrios.append(r);
            }
        }
    } else {
        QString regions("eu us ss uk wor jp au ame de cus cn kr asi br sp fr "
                        "gr it no dk nz nl pl ru se tw ca");
        for (const auto &r : regions.split(" ")) {
            if (!config.regionPrios.contains(r)) {
                config.regionPrios.append(r);
            }
        }
    }
}

void Skyscraper::setLangPrios() {
    // Load single custom lang
    if (!config.lang.isEmpty()) {
        config.langPrios.append(config.lang);
    }

    // Load custom lang prioritizations
    if (!config.langPriosStr.isEmpty()) {
        for (const auto &lang : config.langPriosStr.split(",")) {
            QString l = lang.trimmed();
            if (!config.langPrios.contains(l)) {
                config.langPrios.append(l);
            }
        }
    } else {
        QStringList dfltLangs = {"en", "de", "fr", "es"};
        for (const auto &l : dfltLangs) {
            if (!config.langPrios.contains(l)) {
                config.langPrios.append(l);
            }
        }
    }
}

// --- Console colors ---
// Black        0;30     Dark Gray     1;30
// Red          0;31     Light Red     1;31
// Green        0;32     Light Green   1;32
// Brown/Orange 0;33     Yellow        1;33
// Blue         0;34     Light Blue    1;34
// Purple       0;35     Light Purple  1;35
// Cyan         0;36     Light Cyan    1;36
// Light Gray   0;37     White         1;37
