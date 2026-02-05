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

#include "skyscraper.h"

#include "abstractfrontend.h"
#include "attractmode.h"
#include "batocera.h"
#include "cli.h"
#include "compositor.h"
#include "config.h"
#include "emulationstation.h"
#include "esde.h"
#include "pegasus.h"
#include "settings.h"
#include "strtools.h"

#include <QDebug>
#include <QDirIterator>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QProcess>
#include <QSettings>
#include <QStorageInfo>
#include <QStringBuilder>
#include <QThread>
#include <QTimer>
#include <iostream>

Skyscraper::Skyscraper(const QString &currentDir) {
    qRegisterMetaType<GameEntry>("GameEntry");
    manager = QSharedPointer<NetManager>(new NetManager());
    config.currentDir = currentDir;

    printf("%s", StrTools::getVersionHeader().toStdString().c_str());
}

Skyscraper::~Skyscraper() { frontend->deleteLater(); }

void Skyscraper::run() {
    // called after loadConfig()
    if (config.platform.isEmpty()) {
        if (config.inputFolderNotMain || config.cacheFolderNotMain) {
            printf("\033[1;33m'The hat trick never works!'\033[0m You have set "
                   "either the input folder or cache folder outside the [main] "
                   "configuration section. Paired with the ALL cache command "
                   "this may produce inconsistent results. Use the command "
                   "with a explicit platform (-p) or remove the setting(s). "
                   "Quitting...\n");
            exit(0);
        }
        if (config.cacheOptions == "purge:all") {
            Cache::purgeAllOnAllPlatforms(config, this);
            exit(0);
        } else if (config.cacheOptions.startsWith("report:missing=")) {
            bool ok = Cache::reportAllPlatform(config, this);
            if (ok) {
                exit(0);
            } else {
                printf("Premature end of Skyscraper run.\n");
                exit(1);
            }
        } else if (config.cacheOptions == "vacuum") {
            Cache::vacuumAllPlatform(config, this);
            exit(0);
        } else if (config.cacheOptions == "validate") {
            Cache::validateAllPlatform(config, this);
            exit(0);
        }
    }

    cacheScrapeMode = config.scraper == "cache";
    generateGamelist = cacheScrapeMode && !config.pretend;
    printf("Platform:         '\033[1;32m%s\033[0m'\n",
           config.platform.toStdString().c_str());
    printf("Scraping module:  '\033[1;32m%s\033[0m'\n",
           config.scraper.toStdString().c_str());
    if (cacheScrapeMode && config.cacheOptions.isEmpty()) {
        printf("Frontend:         '\033[1;32m%s\033[0m'\n",
               config.frontend.toStdString().c_str());
        if (!config.frontendExtra.isEmpty()) {
            printf("Extra:            '\033[1;32m%s\033[0m'\n",
                   config.frontendExtra.toStdString().c_str());
        }
    }
    printf("Input folder:     '\033[1;32m%s\033[0m'\n",
           Config::pathToCStr(config.inputFolder));
    printf("Game list folder: '\033[1;32m%s\033[0m'\n",
           Config::pathToCStr(config.gameListFolder));
    if (cacheScrapeMode && config.cacheOptions.isEmpty()) {
        printf("Media folder:     '\033[1;32m%s\033[0m'\n",
               Config::pathToCStr(config.mediaFolder));
        printf("  Covers folder:  '├── \033[1;32m%s\033[0m'\n",
               mediaSubFolderCStr(config.coversFolder));
        printf("  Screenshots:    '├── \033[1;32m%s\033[0m'\n",
               mediaSubFolderCStr(config.screenshotsFolder));
        printf("  Wheels:         '├── \033[1;32m%s\033[0m'\n",
               mediaSubFolderCStr(config.wheelsFolder));
        printf("  Marquees:       '├── \033[1;32m%s\033[0m'\n",
               mediaSubFolderCStr(config.marqueesFolder));
        bool notLast = config.videos || config.manuals || config.backcovers ||
                       config.fanart;
        printf("  Textures:       '%s── \033[1;32m%s\033[0m'\n",
               notLast ? "├" : "└", mediaSubFolderCStr(config.texturesFolder));
        if (config.videos) {
            notLast = config.manuals || config.backcovers || config.fanart;
            printf("  Videos:         '%s── \033[1;32m%s\033[0m'\n",
                   notLast ? "├" : "└",
                   mediaSubFolderCStr(config.videosFolder));
        }
        // config.*Folder are not empty on frontends supporting that media
        if (config.manuals && !config.manualsFolder.isEmpty()) {
            notLast = config.backcovers || config.fanart;
            printf("  Manuals:        '%s── \033[1;32m%s\033[0m'\n",
                   notLast ? "├" : "└",
                   mediaSubFolderCStr(config.manualsFolder));
        }
        if (config.fanart && !config.fanartsFolder.isEmpty()) {
            notLast = config.backcovers;
            printf("  Fanarts:        '%s── \033[1;32m%s\033[0m'\n",
                   notLast ? "├" : "└",
                   mediaSubFolderCStr(config.fanartsFolder));
        }
        if (config.backcovers && !config.backcoversFolder.isEmpty()) {
            notLast = false;
            printf("  Backcovers:     '%s── \033[1;32m%s\033[0m'\n",
                   notLast ? "├" : "└",
                   mediaSubFolderCStr(config.backcoversFolder));
        }
    }
    printf("Cache folder:     '\033[1;32m%s\033[0m'\n",
           Config::pathToCStr(config.cacheFolder));
    if (config.scraper == "import") {
        printf("Import folder:    '\033[1;32m%s\033[0m'\n",
               Config::pathToCStr(config.importFolder));
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

    cache = QSharedPointer<Cache>(new Cache(config.cacheFolder));
    if (cacheScrapeMode || cache->createFolders(config.scraper)) {
        if (!cache->read() && cacheScrapeMode) {
            printf("No resources for this platform found in the resource "
                   "cache ('%s'). Please verify the path of the cache or "
                   "specify a scraping module with '-s' to gather some "
                   "resources before trying to generate a game list. Check "
                   "all available modules with '--help'.\n",
                   config.cacheFolder.toStdString().c_str());
            exit(1);
        }
    } else {
        printf("Couldn't create cache folders, please check folder "
               "permissions and try again...\n");
        exit(1);
    }

    if (config.verbosity || config.cacheOptions == "show") {
        cache->showStats(config.cacheOptions == "show" ? 2 : config.verbosity);
        if (config.cacheOptions == "show") {
            exit(0);
        }
    }
    if (config.cacheOptions.startsWith("purge:") ||
        config.cacheOptions == "vacuum") {
        bool success = true;
        if (config.cacheOptions == "purge:all") {
            success = cache->purgeAllOnSinglePlatform(config.unattend ||
                                                      config.unattendSkip);
        } else if (config.cacheOptions == "vacuum") {
            success = cache->vacuumResources(
                config.inputFolder, getPlatformFileExtensions(),
                config.verbosity, config.unattend || config.unattendSkip);
        } else if (config.cacheOptions.startsWith("purge:")) {
            success = cache->purgeResources(config.cacheOptions);
        }
        if (success) {
            state = NO_INTR; // Ignore ctrl+c
            cache->write();
            state = SINGLE;
        }
        exit(0);
    }
    if (config.cacheOptions.startsWith("report:")) {
        cache->assembleReport(config, getPlatformFileExtensions());
        exit(0);
    }
    if (config.cacheOptions == "validate") {
        cache->validate();
        state = NO_INTR;
        cache->write();
        state = SINGLE;
        exit(0);
    }
    if (config.cacheOptions.startsWith("merge:")) {
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
            state = NO_INTR;
            cache->write();
            state = SINGLE;
        } else {
            printf("Path to merge from '%s' does not exist or is not a path, "
                   "can't continue...\n",
                   absMergeCacheFilePath.toStdString().c_str());
            exit(1);
        }
        exit(0);
    }

    // remaining cache subcommand check
    bool cacheEditCmd = config.cacheOptions.startsWith("edit");
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
            state = NO_INTR;
            cache->write();
        } else {
            printf("Catched Ctrl-C: No changes persisted!\n");
        }
        state = SINGLE;
        exit(0);
    }
    state = SINGLE;

    gameListFileString =
        Config::concatPath(config.gameListFolder, config.gameListFilename);

    QFile gameListFile(gameListFileString);

    if (generateGamelist && config.gameListBackup) {
        QString gameListBackup =
            gameListFile.fileName() + "-" +
            QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        printf("Game list backup saved to '\033[1;33m%s\033[0m'\n",
               gameListBackup.toStdString().c_str());
        gameListFile.copy(gameListBackup);
    }

    if (generateGamelist && !config.unattend && !config.unattendSkip &&
        gameListFile.exists()) {
        std::string userInput = "";
        printf("\033[1;34m'\033[0m\033[1;33m%s\033[0m\033[1;34m' already "
               "exists, do you want to overwrite it\033[0m (y/N)? ",
               config.gameListFilename.toStdString().c_str());
        getline(std::cin, userInput);
        if (userInput != "y" && userInput != "Y") {
            printf("User chose not to overwrite, now exiting...\n");
            exit(0);
        }
        printf("Checking if '\033[1;33m%s\033[0m' is writable?... ",
               config.gameListFilename.toStdString().c_str());

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
            if (!config.unattend && cliFiles.isEmpty() && frontend->canSkip()) {
                std::string userInput = "";
                if (config.unattendSkip) {
                    userInput = "y";
                } else {
                    printf("\033[1;34mDo you want to skip already existing "
                           "game list entries\033[0m (y/N)? ");
                    getline(std::cin, userInput);
                }
                if ((userInput == "y" || userInput == "Y")) {
                    frontend->skipExisting(gameEntries, queue);
                }
            }
            printf("\n");
        } else {
            printf("\033[1;33mNot found or unsupported!\033[0m\n\n");
        }
    }

    totalFiles = queue->length();
    if (totalFiles == 0) {
        QString extraInfo =
            generateGamelist
                ? "in cache"
                : "matching these extensions '" +
                      getPlatformFileExtensions().split(' ').join(", ") + "'";
        QString unattendSkipStr = "";
        if (!config.unattend && cliFiles.isEmpty()) {
            unattendSkipStr =
                "\nMaybe you have opted to skip existing gamelist "
                "entries (see config: unattendSkip) from this "
                "\nSkyscraper run and there is none remaining to ";
            unattendSkipStr =
                unattendSkipStr % ((generateGamelist)
                                       ? "generate a gamelist entry for."
                                       : "scrape.");
        }
        printf("\nNo files to process %s for platform "
               "'%s'.\nCheck configured and existing file extensions and cache "
               "content.%s\n\n\033[1;33mSkyscraper came to an untimely "
               "end.\033[0m\n\n",
               extraInfo.toStdString().c_str(),
               config.platform.toStdString().c_str(),
               unattendSkipStr.toStdString().c_str());
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
                "Please check '--help' for more info.\n\nNow quitting...\n\n",
                config.romLimit);
            exit(0);
        }
    }

    if (!Compositor::preCheckArtworkXml(config.artworkXml)) {
        printf("Parsing artwork XML from '%s', failed, see above."
               "Check the file for errors. Now exiting...\n",
               config.artworkConfig.toStdString().c_str());
        exit(1);
    }

    if (!generateGamelist) {
        printf("Starting scraping run on \033[1;32m%d\033[0m files using "
               "\033[1;32m%d\033[0m threads.\nSit back, relax and let me do "
               "the work! :)\n\n",
               totalFiles, config.threads);
    }

    createMediaOutFolders();

    timer.start();
    currentFile = 1;

    QList<QThread *> threadList;
    for (int curThread = 1; curThread <= config.threads; ++curThread) {
        QThread *thread = new QThread;
        ScraperWorker *worker =
            new ScraperWorker(queue, cache, frontend, manager, config,
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
        printf("Input folder '\033[1;31m%s\033[0m' doesn't exist or can't be "
               "accessed by current user. Please check path and permissions.\n",
               inputDir.absolutePath().toStdString().c_str());
        exit(1);
    }

    // always create gamelist folder
    setFolder(generateGamelist, config.gameListFolder);
    // check importFolder is existing when no gamelist is created (e.g., import
    // scraping), but never create
    setFolder(!generateGamelist, config.importFolder, false);

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
            startAt.setFile(config.inputFolder + "/" + config.startAt);
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
            endAt.setFile(config.inputFolder + "/" + config.endAt);
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
            QList<QFileInfo> subFiles = inputDir.entryInfoList();
            if (config.platform == "scummvm" &&
                config.frontend == "emulationstation") {
                // special case: avoid having files like
                // .../scummvm/blarf.svm/blarf.svm added as game (as the folder
                // blarf.svm/ acts as ROM file already)
                for (auto i = subFiles.begin(), end = subFiles.end(); i != end;
                     ++i) {
                    if (subdir.contains("/" % (*i).fileName())) {
                        subFiles.erase(i);
                        break;
                    }
                }
            }
            queue->append(subFiles);
            if (config.verbosity > 0 && subFiles.size() > 0) {
                printf("Adding matching files from subdir: '%s'\n",
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

void Skyscraper::createMediaOutFolders() {
    // make and check iff gamelist is outputted
    setFolder(generateGamelist, config.coversFolder, generateGamelist);
    setFolder(generateGamelist, config.screenshotsFolder, generateGamelist);
    setFolder(generateGamelist, config.wheelsFolder, generateGamelist);
    setFolder(generateGamelist, config.marqueesFolder, generateGamelist);
    setFolder(generateGamelist, config.texturesFolder, generateGamelist);
    if (config.videos) {
        setFolder(generateGamelist, config.videosFolder, generateGamelist);
    }
    if (config.manuals) {
        setFolder(generateGamelist, config.manualsFolder, generateGamelist);
    }
    if (config.fanart) {
        setFolder(generateGamelist, config.fanartsFolder, generateGamelist);
    }
    if (config.backcovers) {
        setFolder(generateGamelist, config.backcoversFolder, generateGamelist);
    }
}

void Skyscraper::setFolder(const bool generateGamelist, QString &outFolder,
                           const bool createMissingFolder) {
    if (!outFolder.isEmpty()) {
        QDir dir(outFolder);
        if (generateGamelist) {
            checkForFolder(dir, createMissingFolder);
        }
        outFolder = dir.absolutePath();
    }
}

void Skyscraper::checkForFolder(QDir &folder, bool create) {
    if (!folder.exists()) {
        if (create) {
            if (!folder.mkpath(folder.absolutePath())) {
                printf(
                    "Create folder '\033[1;31m%s\033[0m' failed! Please "
                    "check path and filesystem permissions, now exiting...\n",
                    folder.absolutePath().toStdString().c_str());
                exit(1);
            }
        } else {
            printf("Folder '%s' doesn't exist, can't continue...\n",
                   folder.absolutePath().toStdString().c_str());
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
        if (skippedFile.open(QIODevice::Append)) {
            skippedFile.write(entry.absoluteFilePath.toUtf8() + "\n");
            skippedFile.close();
        } else {
            qWarning() << "File not writeable" << skippedFileString;
        }
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

    const qint64 spaceLimit = 200 * 1024 * 1024;
    if (config.spaceCheck) {
        QString storage;
        if (config.scraper == "cache" && !config.pretend &&
            QStorageInfo(QDir(config.screenshotsFolder)).bytesFree() <
                spaceLimit) {
            storage = "media export";
        } else if (QStorageInfo(QDir(config.cacheFolder)).bytesFree() <
                   spaceLimit) {
            storage = "resource cache";
        }
        if (!storage.isEmpty()) {

            printf("\033[1;31mYou have very little disk space left on the "
                   "Skyscraper %s storage, please free up some space "
                   "and try again. Now aborting...\033[0m\n\n",
                   storage.toStdString().c_str());
            printf("Note! You can disable this check by setting "
                   "'spaceCheck=\"false\"' in the '[main]' section of "
                   "config.ini.\n\n");
            // By clearing the queue here we basically tell Skyscraper to stop
            // and quit nicely
            config.pretend = true;
            queue->clearAll();
        }
    }
}

void Skyscraper::checkThreads() {
    QMutexLocker locker(&checkThreadMutex);

    doneThreads++;
    if (doneThreads != config.threads)
        return;

    if (!config.pretend && config.scraper == "cache") {
        printf("\033[1;34m---- Game list generation run completed! YAY! "
               "----\033[0m\n");
        state = NO_INTR;
        cache->write(true);
        state = SINGLE;
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
            state = NO_INTR;
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
        state = NO_INTR;
        cache->write();
        state = SINGLE;
    }

    if (!generateGamelist || totalFiles > 0) {
        printf("\033[1;34m---- And here are some neat stats :) ----\033[0m\n");
    }
    if (!generateGamelist) {
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
            QString skippedFn = Config::concatPath(
                Config::getSkyFolder(Config::SkyFolderType::LOG),
                skippedFileString);
            printf(" (Filenames saved to '\033[1;33m%s\033[0m')",
                   Config::pathToCStr(skippedFn));
        }
        printf("\n\n");
    }

    // All done, now clean up and exit to terminal
    cleanUp();
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
        } else {
            printf("File '\033[1;32m%s\033[0m' can not be read.\n\nPlease "
                   "verify the file permissions and try again...\n",
                   fnInfo.absoluteFilePath().toStdString().c_str());
            exit(1);
        }
    } else {
        printf("File '\033[1;32m%s\033[0m' does not exist.\n\nPlease "
               "verify the filename and try again...\n",
               fnInfo.absoluteFilePath().toStdString().c_str());
        exit(1);
    }
    return fileList;
}

void Skyscraper::loadConfig(const QCommandLineParser &parser) {
    // called before run()
    QString iniFile = parser.isSet("c") ? parser.value("c") : "config.ini";
    QString absIniFile = Config::makeAbsolutePath(
        parser.isSet("c") ? config.currentDir : Config::getSkyFolder(),
        iniFile);
    absIniFile = Config::lexicallyNormalPath(absIniFile);
    if (!QFileInfo(absIniFile).exists()) {
        printf(
            "\nWARNING! Provided config file '\033[1;33m%s\033[0m' does not "
            "exist.\nSkyscraper will use default configuration values...\n\n",
            iniFile.toStdString().c_str());
    }
    config.configFile = absIniFile;
    QSettings settings(absIniFile, QSettings::IniFormat);

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
    QStringList scrapers = {"arcadedb",   "cache",          "esgamelist",
                            "gamebase",   "igdb",           "import",
                            "mobygames",  "openretro",      "screenscraper",
                            "thegamesdb", "worldofspectrum"};
    if (parser.isSet("s")) {
        QString _scraper = parser.value("s");
        if (_scraper == "tgdb") {
            _scraper = "thegamesdb";
        } else if (_scraper == "wos" || _scraper == "zxinfo") {
            /* not using zxinfo bc. backward compability, esp. for resource
             * cache */
            _scraper = "worldofspectrum";
        }
        if (scrapers.contains(_scraper)) {
            config.scraper = _scraper;
        } else {
            printf("\033[1;31mBummer!\033[0m Unknown scrapingmodule "
                   "'\033[1;31m%s\033[0m'. Known scrapers are: %s.\nHint: Try "
                   "TAB-completion to avoid typos.\n",
                   _scraper.toStdString().c_str(),
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

    if (config.frontend == "emulationstation" ||
        config.frontend == "retrobat") {
        frontend = QSharedPointer<AbstractFrontend>(new EmulationStation());
    } else if (config.frontend == "attractmode") {
        frontend = QSharedPointer<AbstractFrontend>(new AttractMode());
    } else if (config.frontend == "pegasus") {
        frontend = QSharedPointer<AbstractFrontend>(new Pegasus());
    } else if (config.frontend == "esde") {
        frontend = QSharedPointer<AbstractFrontend>(new Esde());
    } else if (config.frontend == "batocera") {
        frontend = QSharedPointer<AbstractFrontend>(new Batocera());
    }

    // Choose default scraper if none has been set yet
    if (config.scraper.isEmpty()) {
        config.scraper = "cache";
    }

    frontend->setConfig(&config);
    frontend->checkReqs();

    // Fallback to defaults if they aren't already set, find the rest in
    // settings.h
    if (config.frontend != "batocera") {
        if (!inputFolderSet)
            config.inputFolder = frontend->getInputFolder();
        if (!gameListFolderSet)
            config.gameListFolder = frontend->getGameListFolder();
    } else {
        if (!gameListFolderSet)
            config.gameListFolder = frontend->getGameListFolder();
        if (!inputFolderSet)
            config.inputFolder = frontend->getInputFolder();
    }
    if (!mediaFolderSet) {
        if (config.frontend == "esde" || config.frontend == "batocera") {
            config.mediaFolder = frontend->getMediaFolder();
        } else {
            // defaults to <gamelistfolder>/[.]media/
            QString mf = "media";
            if (config.mediaFolderHidden) {
                mf = "." + mf;
            }
            config.mediaFolder = Config::concatPath(config.gameListFolder, mf);
        }
    }
    Config::expandHomePath(config.inputFolder);
    Config::expandHomePath(config.mediaFolder);

    // defaults are always absolute, thus input- and mediafolder will be
    // unchanged by these calls.
    // gamelistfolder is absolute by now.
    // the other two may be relative or absolute.
    if (config.frontend == "pegasus" || config.frontend == "batocera") {
        QString last = config.gameListFolder.split("/").last();
        config.inputFolder = removeSurplusPlatformPath(config.platform, last,
                                                       config.inputFolder);
        config.mediaFolder = removeSurplusPlatformPath(config.platform, last,
                                                       config.mediaFolder);
        config.inputFolder =
            Config::makeAbsolutePath(config.gameListFolder, config.inputFolder);
        config.mediaFolder =
            Config::makeAbsolutePath(config.gameListFolder, config.mediaFolder);
    } else {
        QFileInfo inputDirFileInfo = QFileInfo(config.inputFolder);
        if (inputDirFileInfo.isRelative()) {
            printf("\033[1;31mBummer!\033[0m The parameter 'inputFolder' is "
                   "provided as relative path which is not valid for this "
                   "frontend. Provide the input folder as absolute path to "
                   "remediate. Now quitting...\n");
            exit(1);
        }
        QString last = config.inputFolder.split("/").last();
        config.gameListFolder = removeSurplusPlatformPath(
            config.platform, last, config.gameListFolder);
        config.mediaFolder = removeSurplusPlatformPath(config.platform, last,
                                                       config.mediaFolder);
        if (config.inputFolder != config.gameListFolder &&
            inputDirFileInfo.canonicalFilePath() ==
                frontend->getInputFolder()) {
            // edge case: user has provided a inputFolder symlink which expands
            // to frontend->getInputFolder() and mediafolder is relative.
            // Rationale: ES expects relative paths to media files defined in
            // gamelist.xml relative to frontend->getInputFolder()
            config.inputFolder = frontend->getInputFolder();
        }
        config.mediaFolder =
            Config::makeAbsolutePath(config.inputFolder, config.mediaFolder);
    }
    config.inputFolder = Config::lexicallyNormalPath(config.inputFolder);

    if (config.platform.isEmpty() && !config.cacheOptions.isEmpty()) {
        return; // cache option to be applied to all platform
    }

    if (!QFile::exists(config.inputFolder)) {
        printf("\033[1;31mBummer!\033[0m The provided input folder "
               "'\033[1;31m%s\033[0m' does not exist.\nFix your setup. Now "
               "quitting...\n",
               config.inputFolder.toStdString().c_str());
        exit(1);
    }
    config.mediaFolder = Config::lexicallyNormalPath(config.mediaFolder);

    config.gameListFilename = frontend->getGameListFileName();

    // only resolve after config.mediaFolder is set
    config.coversFolder = frontend->getCoversFolder();
    config.screenshotsFolder = frontend->getScreenshotsFolder();
    config.wheelsFolder = frontend->getWheelsFolder();
    config.marqueesFolder = frontend->getMarqueesFolder();
    config.texturesFolder = frontend->getTexturesFolder();
    config.videosFolder = frontend->getVideosFolder();
    config.manualsFolder = frontend->getManualsFolder();
    config.fanartsFolder = frontend->getFanartsFolder();
    config.backcoversFolder = frontend->getBackcoversFolder();

    if (config.importFolder.isEmpty()) {
        config.importFolder =
            Config::getSkyFolder(Config::SkyFolderType::IMPORT);
    }
    // If platform subfolder exists for import path, use it
    QDir importFolder(config.importFolder);
    if (importFolder.exists(config.platform)) {
        config.importFolder =
            Config::concatPath(config.importFolder, config.platform);
    }

    // Set minMatch to 0 for cache, arcadedb and screenscraper
    // We know these results are always accurate
    if (config.minMatchSet == false && config.isMatchOneScraper()) {
        config.minMatch = 0;
    }

    skippedFileString =
        QString("skipped-%1-%2.txt")
            .arg(config.platform)
            .arg(config.scraper != "worldofspectrum" ? config.scraper
                                                     : "zxinfo");

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
            romPath = normalizePath(requestedFileInfo);
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
        printf("Filename: '\033[1;33m%s\033[0m' requested either on command "
               "line or with '--includefrom' not found in(side) the input "
               "directory '\033[1;33m%s\033[0m'!\n\nPlease verify the filename "
               "and try again...\n",
               requestedFileInfo.fileName().toStdString().c_str(),
               config.inputFolder.toStdString().c_str());
        exit(1);
    }

    // Add query only if a single filename was passed on command line
    if (parser.isSet("query")) {
        if (QStringList({"esgamelist", "import", "cache"})
                .contains(config.scraper)) {
            printf("'--query' can not be applied for gamelist creation or for "
                   "this scraper. Now quitting...\n");
            exit(0);
        }
        if (cliFiles.length() == 1) {
            config.searchName = parser.value("query");
            config.threads = 1;
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

    if (!config.userCreds.isEmpty()) {
        QStringList userCreds = config.userCreds.split(":");
        if (userCreds.length() == 2) {
            config.user = userCreds.at(0);
            config.password = userCreds.at(1);
        } else if (userCreds.length() == 1) {
            // API key
            config.password = userCreds.at(0);
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
    } else if (config.scraper == "mobygames") {
        if (config.threads != 1) {
            printf("\033[1;33mForcing one thread to accomodate limits in "
                   "MobyGames scraping module. Also be aware that MobyGames "
                   "has a request limit of 720 requests per hour, 12 per "
                   "minute, for a Hobbyist subscription.\033[0m\n\n");
            config.threads = 1;
        }
        if (config.password.isEmpty()) {
            printf("The MobyGames scraping module requires an API key to work. "
                   "More info: 'https://gemba.github.io/skyscraper/"
                   "SCRAPINGMODULES#mobygames'\n");
            exit(1);
        }
    } else if (config.scraper == "screenscraper") {
        prepareScreenscraper(netComm, q);
    } else if (config.scraper == "gamebase") {
        config.threads = 1;
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
        // max. 4 request per sec
        printf("\033[1;33mAdjusting to %d threads to accomodate limits in "
               "the IGDB API\033[0m\n\n",
               config.threads);
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
        // failsafe
        tokenData = "user;token;0";
    }
    bool updateToken = false;
    if (config.user != tokenData.split(';').at(0)) {
        updateToken = true;
    }
    // convert token validity-end-date to a duration
    qlonglong validDuration = tokenData.split(';').at(2).toLongLong() -
                              QDateTime::currentSecsSinceEpoch();
    // rectify bug in Skyscraper < 3.18.0 (expiry of token calculated to long).
    // https://github.com/muldjord/skyscraper/blob/03d8d657d1ea3dfc5d3b1047922b76971bdf5ff5/src/skyscraper.cpp#L1736-L1738
    // igdb token is usually 60 days valid, if it is much longer valid, force
    // refresh to rectify bug
    if (validDuration > 3600 * 24 * 60) {
        updateToken = true;
    }
    // two days, should be plenty for a scraping run, below that refresh token
    if (validDuration < 3600 * 24 * 2) {
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
            printf("...token acquired, ready to scrape!\n");
            qlonglong tokenEol = QDateTime::currentSecsSinceEpoch() +
                                 jsonObj["expires_in"].toInt();
            if (tokenFile.open(QIODevice::WriteOnly)) {
                tokenFile.write(config.user.toUtf8() + ";" +
                                config.igdbToken.toUtf8() + ";" +
                                QString::number(tokenEol).toUtf8());
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
        printf("...cached token still valid, ready to scrape!\n");
    }
    printf("\n");
}

void Skyscraper::prepareScreenscraper(NetComm &netComm, QEventLoop &q) {
    const int threadsFailsafe = 1; // Don't change! This limit was set by
                                   // request from ScreenScraper
    if (config.user.isEmpty() || config.password.isEmpty()) {
        if (config.threads > 1) {
            config.threads = threadsFailsafe;
            printf("\033[1;33mForcing %d thread as this is the anonymous "
                   "limit in the ScreenScraper scraping module. Sign up for "
                   "an account at https://www.screenscraper.fr and support "
                   "them to gain more threads. Then use the credentials with "
                   "Skyscraper using the '-u user:password' command line "
                   "option or by setting 'userCreds=\"user:password\"' in "
                   "'%s/config.ini'.\033[0m\n\n",
                   config.threads,
                   Config::getSkyFolder().toStdString().c_str());
        }
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
            if (allowedThreads > 0) {
                if (config.threadsSet && config.threads < allowedThreads) {
                    printf(
                        "User is allowed %d thread%s, but user has set thread "
                        "value manually to %d: Using the lower value.\n\n",
                        allowedThreads, allowedThreads > 1 ? "s" : "",
                        config.threads);
                } else {
                    config.threads = allowedThreads;
                    printf("Setting threads to \033[1;32m%d\033[0m as "
                           "allowed for your Screenscraper account.\n\n",
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

QString &Skyscraper::removeSurplusPlatformPath(const QString &platform,
                                               const QString &lastPath,
                                               QString &sourcePath) {
    if (sourcePath.startsWith("./") && lastPath == platform) {
        const QString subPath = "/" % lastPath;
        if (sourcePath.endsWith(subPath)) {
            qDebug() << "pre  sourcePath" << sourcePath;
            sourcePath.replace(subPath, "");
            qDebug() << "post sourcePath" << sourcePath;
        }
    }
    return sourcePath;
}

void Skyscraper::cleanUp() {
    QStringList mediaDirs = {config.coversFolder,    config.screenshotsFolder,
                             config.wheelsFolder,    config.marqueesFolder,
                             config.texturesFolder,  config.videosFolder,
                             config.manualsFolder,   config.fanartsFolder,
                             config.backcoversFolder};
    // remove folders that are empty
    for (const auto &f : mediaDirs) {
        if (!f.isEmpty()) {
            // qDebug() << "rmdir " << f;
            QDir dir(f);
            dir.rmdir(f);
        }
    }
}

const char *Skyscraper::mediaSubFolderCStr(QString &in) {
    QString ret = Config::lexicallyRelativePath(config.mediaFolder, in);
    return ret.toUtf8().constData();
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
