/*
 *  This file is part of skyscraper.
 *  Copyright 2023 Gemba @ GitHub
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

#include "settings.h"

#include "cache.h"
#include "cli.h"
#include "config.h"
#include "platform.h"

#include <QDebug>
#include <QFileInfo>
#include <QStringBuilder>
#ifdef __MINGW32__
#include <experimental/filesystem>
#else
#include <filesystem>
#endif

static inline bool isArcadePlatform(const QString &platform) {
    const QStringList arcadePlatforms = {"arcade",        "fba",
                                         "mame-advmame",  "mame-libretro",
                                         "mame-mame4all", "neogeo"};
    return arcadePlatforms.contains(platform);
}

RuntimeCfg::RuntimeCfg(Settings *config, const QCommandLineParser *parser) {
    this->config = config;
    this->parser = parser;
}

RuntimeCfg::~RuntimeCfg(){};

void RuntimeCfg::reportInvalidPlatform() {
    if (parser->isSet("p")) {
        printf("\033[1;31mUnknown platform '%s' provided. \033[0m",
               parser->value("p").toUtf8().constData());
    }
    printf("\033[1;31mPlease set a valid platform with '-p "
           "<PLATFORM>'\nCheck '--help' for a list of supported "
           "platforms. Qutting.\n\033[0m");
}

void RuntimeCfg::applyConfigIni(CfgType type, QSettings *settings,
                                bool &inputFolderSet, bool &gameListFolderSet,
                                bool &mediaFolderSet) {

    // Check for command line platform here, since we need it for 'platform'
    // config.ini entries
    if (config->platform.isEmpty()) {
        QStringList plafs = Platform::get().getPlatforms();
        if (parser->isSet("p") &&
            plafs.contains(parser->value("p").split('_').first())) {
            /* '_' is seen as a subcategory of the selected platform */
            config->platform = parser->value("p");
        } else if (type == CfgType::MAIN && settings->contains("platform") &&
                   plafs.contains(settings->value("platform").toString())) {
            // config.ini may set platform= in [main]
            config->platform = settings->value("platform").toString();
        } else {
            bool isCacheOK =
                parser->isSet("cache") &&
                Cache::isCommandValidOnAllPlatform(parser->value("cache"));
            bool isFlagsOK =
                parser->isSet("flags") && parseFlags().contains("help");

            if (!isCacheOK && !isFlagsOK && !parser->isSet("hint")) {
                reportInvalidPlatform();
                exit(1);
            }
        }
    }

    config->arcadePlatform = isArcadePlatform(config->platform);

    // get all enabled/set keys of this section
    QStringList keys = settings->childKeys();
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QSet<QString> cfgIniKeys(keys.begin(), keys.end());
#else
    QSet<QString> cfgIniKeys(keys.toSet());
#endif
    // get allowed keys for this section type and retain intersection
    QSet<QString> allowedKeys = getKeys(type);
    QSet<QString> intersect = allowedKeys.intersect(cfgIniKeys);

    bool frontendKey = false;
    if (intersect.contains("frontend")) {
        intersect.remove("frontend");
        frontendKey = true;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QStringList retained(intersect.begin(), intersect.end());
#else
    QStringList retained;
    QSetIterator<QString> iter(intersect);
    while (iter.hasNext()) {
        retained.append(iter.next());
    }
#endif
    retained.sort();

    if (type == CfgType::MAIN && config->frontend.isEmpty()) {
        if (frontendKey) {
            // get from config.ini
            retained.insert(0, "frontend");
        } else {
            // not present in config.ini and not yet set with -f
            // set default
            config->frontend = "emulationstation";
        }
    }

    // check if config.ini has not-applicable keys in section
    QSet<QString> invalid = cfgIniKeys.subtract(allowedKeys);
    if (!invalid.isEmpty()) {
        QString section;
        switch (type) {
        case CfgType::MAIN:
            section = "main";
            break;
        case CfgType::PLATFORM:
            section = config->platform;
            break;
        case CfgType::FRONTEND:
            section = config->frontend;
            break;
        case CfgType::SCRAPER:
            section = config->scraper;
            break;
        default:;
        }
        qInfo() << "Section type" << type << "[" << section << "]"
                << "has surplus key(s) (=ignored): " << invalid;
    }

    for (auto k : retained) {
        QString conv = params[k].first;
        QVariant ss = settings->value(k);
        if (conv == "str") {
            QString v = ss.toString();
            if (k == "addExtensions") {
                config->addExtensions = v;
                continue;
            }
            if (k == "artworkXml") {
                if (!v.isEmpty() && QFileInfo(v).isRelative()) {
                    config->artworkConfig =
                        concatPath(Config::getSkyFolder(), v);
                } else {
                    config->artworkConfig = v;
                }
                continue;
            }
            if (k == "cacheFolder") {
                config->cacheFolder = (type == CfgType::MAIN)
                                          ? concatPath(v, config->platform)
                                          : v;
                continue;
            }
            if (k == "emulator") {
                if (config->frontend == "attractmode") {
                    config->frontendExtra = v;
                } else {
                    printf("\033[1;33mParameter emulator is ignored. Only "
                           "applicable with frontend=attractmode.\n\033[0m");
                }
                continue;
            }
            if (k == "endAt") {
                config->endAt = v;
                continue;
            }
            if (k == "excludeFrom") {
                config->excludeFrom = v;
                continue;
            }
            if (k == "excludePattern") {
                config->excludePattern = v;
                continue;
            }
            if (k == "extensions") {
                config->extensions = v;
                continue;
            }
            if (k == "gameListFolder") {
                config->gameListFolder = (type == CfgType::MAIN ||
                                          type == CfgType::FRONTEND /* #68 */)
                                             ? concatPath(v, config->platform)
                                             : v;
                gameListFolderSet = true;
                continue;
            }
            if (k == "frontend") {
                if (!validateFrontend(v)) {
                    exit(1);
                }
                config->frontend = v;
                continue;
            }
            if (k == "importFolder") {
                config->importFolder = v;
                continue;
            }
            if (k == "gameListVariants") {
                if (config->frontend == "emulationstation") {
                    config->gameListVariants = v;
                } else {
                    printf(
                        "\033[1;33mParameter %s is ignored. Only "
                        "applicable with frontend=emulationstation.\n\033[0m",
                        k.toUtf8().constData());
                }
                continue;
            }
            if (k == "includePattern") {
                config->includePattern = v;
                continue;
            }
            if (k == "includeFrom") {
                config->includeFrom = v;
                continue;
            }
            if (k == "innerBracketsReplace") {
                config->innerBracketsReplace = v;
                continue;
            }
            if (k == "innerParenthesesReplace") {
                config->innerParenthesesReplace = v;
                continue;
            }
            if (k == "inputFolder") {
                config->inputFolder = (type == CfgType::MAIN)
                                          ? concatPath(v, config->platform)
                                          : v;
                inputFolderSet = true;
                continue;
            }
            if (k == "lang") {
                config->lang = v;
                continue;
            }
            if (k == "langPrios") {
                config->langPriosStr = v;
                continue;
            }
            if (k == "launch") {
                if (config->frontend == "pegasus") {
                    config->frontendExtra = v;
                } else {
                    printf("\033[1;33mParameter launch is ignored. Only "
                           "applicable with frontend=pegasus.\n\033[0m");
                }
                continue;
            }
            if (k == "mediaFolder") {
                config->mediaFolder = (type == CfgType::MAIN ||
                                       type == CfgType::FRONTEND /* #68 */)
                                          ? concatPath(v, config->platform)
                                          : v;
                mediaFolderSet = true;
                continue;
            }
            if (k == "nameTemplate") {
                config->nameTemplate = v;
                continue;
            }
            if (k == "platform") {
                config->platform = v;
                continue;
            }
            if (k == "region") {
                config->region = v;
                continue;
            }
            if (k == "regionPrios") {
                config->regionPriosStr = v;
                continue;
            }
            if (k == "scummIni") {
                config->scummIni = v;
                continue;
            }
            if (k == "startAt") {
                config->startAt = v;
                continue;
            }
            if (k == "userCreds") {
                config->userCreds = v;
                continue;
            }
            if (k == "videoConvertCommand") {
                config->videoConvertCommand = v;
                continue;
            }
            if (k == "videoConvertExtension") {
                config->videoConvertExtension = v;
                continue;
            }
        } else if (conv == "bool") {
            bool v = ss.toBool();
            if (k == "brackets") {
                config->brackets = v;
                continue;
            }
            if (k == "cacheCovers") {
                config->cacheCovers = v;
                continue;
            }
            if (k == "cacheMarquees") {
                config->cacheMarquees = v;
                continue;
            }
            if (k == "cacheRefresh") {
                config->refresh = v;
                continue;
            }
            if (k == "cacheResize") {
                config->cacheResize = v;
                continue;
            }
            if (k == "cacheScreenshots") {
                config->cacheScreenshots = v;
                continue;
            }
            if (k == "cacheTextures") {
                config->cacheTextures = v;
                continue;
            }
            if (k == "cacheWheels") {
                config->cacheWheels = v;
                continue;
            }
            if (k == "cropBlack") {
                config->cropBlack = v;
                continue;
            }
            if (k == "addFolders") {
                QStringList allowedFe({"emulationstation", "esde", "retrobat"});
                if (allowedFe.contains(config->frontend)) {
                    config->addFolders = v;
                } else {
                    printf("\033[1;33mParameter %s is ignored. Only applicable "
                           "with frontend %s.\n\033[0m",
                           k.toUtf8().constData(),
                           allowedFe.join(", ").toUtf8().constData());
                }
                continue;
            }
            if (k == "forceFilename") {
                config->forceFilename = v;
                continue;
            }
            if (k == "gameListBackup") {
                config->gameListBackup = v;
                continue;
            }
            if (k == "hints") {
                config->hints = v;
                continue;
            }
            if (k == "interactive") {
                config->interactive = v;
                continue;
            }
            if (k == "ignoreYearInFilename") {
                config->ignoreYearInFilename = v;
                continue;
            }
            if (k == "keepDiscInfo") {
                config->keepDiscInfo = v;
                continue;
            }
            if (k == "mediaFolderHidden") {
                QStringList allowedFe({"emulationstation", "retrobat"});
                if (allowedFe.contains(config->frontend)) {
                    config->mediaFolderHidden = v;
                } else {
                    printf("\033[1;33mParameter %s is ignored. Only applicable "
                           "with frontend %s.\n\033[0m",
                           k.toUtf8().constData(),
                           allowedFe.join(" or ").toUtf8().constData());
                }
                continue;
            }
            if (k == "onlyMissing") {
                config->onlyMissing = v;
                continue;
            }
            if (k == "pretend") {
                config->pretend = v;
                continue;
            }
            if (k == "relativePaths") {
                config->relativePaths = v;
                continue;
            }
            if (k == "skipped") {
                config->skipped = v;
                continue;
            }
            if (k == "spaceCheck") {
                config->spaceCheck = v;
                continue;
            }
            if (k == "subdirs") {
                config->subdirs = v;
                continue;
            }
            if (k == "symlink") {
                config->symlink = v;
                continue;
            }
            if (k == "theInFront") {
                config->theInFront = v;
                continue;
            }
            if (k == "tidyDesc") {
                config->tidyDesc = v;
                continue;
            }
            if (k == "unattend") {
                config->unattend = v;
                continue;
            }
            if (k == "unattendSkip") {
                config->unattendSkip = v;
                continue;
            }
            if (k == "unpack") {
                config->unpack = v;
                continue;
            }
            if (k == "videoPreferNormalized") {
                if (config->scraper == "screenscraper") {
                    config->videoPreferNormalized = v;
                } else {
                    printf(
                        "\033[1;33mParameter videoPreferNormalized is ignored. "
                        "Only applicable to scraper screenscraper\n.\n\033[0m");
                }
                continue;
            }
            if (k == "videos") {
                config->videos = v;
                continue;
            }
            if (k == "manuals") {
                config->manuals = v;
                continue;
            }
        } else if (conv == "int") {
            bool intOk;
            int v = ss.toInt(&intOk);
            if (!intOk) {
                printf("\033[1;31mConversion of %s to integer failed for key "
                       "%s. Please fix in config.ini.\n\033[0m",
                       ss.toString().toUtf8().constData(),
                       k.toUtf8().constData());
                exit(1);
            }
            if (k == "jpgQuality") {
                if (0 < v && v <= 100) {
                    config->jpgQuality = v;
                } else {
                    printf("\033[1;33mValue of %d is out of range and is "
                           "ignored! Consult the documentation.\n\033[0m",
                           v);
                }
                continue;
            }
            if (k == "maxLength") {
                config->maxLength = v;
                continue;
            }
            if (k == "maxFails") {
                if (0 < v && v <= 200) {
                    config->maxFails = v;
                } else {
                    printf("\033[1;33mValue of %d is out of range and is "
                           "ignored! Consult the documentation.\n\033[0m",
                           v);
                }
                continue;
            }
            if (k == "minMatch") {
                QStringList denyScrp({"arcadedb", "cache", "esgamelist",
                                      "import", "screenscraper"});
                if (denyScrp.contains(config->scraper)) {
                    printf("\033[1;33mValue of %s is ignored for scrapers "
                           "%s.\n\033[0m",
                           k.toUtf8().constData(),
                           denyScrp.join(", ").toUtf8().constData());
                } else if (0 < v && v <= 100) {
                    config->minMatch = v;
                    config->minMatchSet = true;
                } else {
                    printf("\033[1;33mValue of %d is out of range and is "
                           "ignored! Consult the documentation.\n\033[0m",
                           v);
                }
                continue;
            }
            if (k == "threads") {
                config->threads = v;
                config->threadsSet = true;
                continue;
            }
            if (k == "verbosity") {
                if (0 < v && v <= 3) {
                    config->verbosity = v;
                } else {
                    printf("\033[1;33mValue of %d is out of range and is "
                           "ignored! Consult the documentation.\n\033[0m",
                           v);
                }
                continue;
            }
            if (k == "videoSizeLimit") {
                config->videoSizeLimit = v * 1000 * 1000;
                continue;
            }
        }
    }
}

void RuntimeCfg::applyCli(bool &inputFolderSet, bool &gameListFolderSet,
                          bool &mediaFolderSet) {
    if (parser->isSet("l") && parser->value("l").toInt() >= 0 &&
        parser->value("l").toInt() <= 10000) {
        config->maxLength = parser->value("l").toInt();
    }
    if (parser->isSet("t") && parser->value("t").toInt() <= 8) {
        config->threads = parser->value("t").toInt();
        config->threadsSet = true;
    }
    if (parser->isSet("e")) {
        config->frontendExtra = parser->value("e");
    }
    if (parser->isSet("i")) {
        config->inputFolder = parser->value("i");
        inputFolderSet = true;
    }
    if (parser->isSet("g")) {
        config->gameListFolder = parser->value("g");
        gameListFolderSet = true;
    }
    if (parser->isSet("o")) {
        config->mediaFolder = parser->value("o");
        mediaFolderSet = true;
    }
    if (parser->isSet("a")) {
        config->artworkConfig = parser->value("a");
        if (QFileInfo(config->artworkConfig).isRelative()) {
            config->artworkConfig =
                concatPath(config->currentDir, config->artworkConfig);
        }
    } else if (config->artworkConfig.isEmpty()) {
        // failsafe: no CLI and no config.ini artworkConfig provided
        config->artworkConfig =
            concatPath(Config::getSkyFolder(), "artwork.xml");
    }
    if (parser->isSet("m") && parser->value("m").toInt() >= 0 &&
        parser->value("m").toInt() <= 100) {
        config->minMatch = parser->value("m").toInt();
        config->minMatchSet = true;
    }
    if (parser->isSet("u")) {
        config->userCreds = parser->value("u");
    }
    if (parser->isSet("d")) {
        config->cacheFolder = parser->value("d");
        if (QFileInfo(config->cacheFolder).isRelative()) {
            config->cacheFolder =
                concatPath(config->currentDir, config->cacheFolder);
        }
    } else if (config->cacheFolder.isEmpty()) {
        // failsafe: no CLI and no config.ini cacheFolder provided
        config->cacheFolder =
            concatPath(Config::getSkyFolder(Config::SkyFolderType::CACHE),
                       config->platform);
    }
    QStringList flags = parseFlags();
    if (flags.contains("help")) {
        Cli::subCommandUsage("flags");
        exit(0);
    } else {
        for (const auto &flag : flags) {
            setFlag(flag);
        }
    }
    if (parser->isSet("addext")) {
        config->addExtensions = parser->value("addext");
    }
    if (parser->isSet("refresh")) {
        config->refresh = true;
    }
    if (parser->isSet("hint")) {
        Cli::showHint();
        exit(0);
    }
    if (parser->isSet("cache")) {
        config->cacheOptions = parser->value("cache");
        if (config->cacheOptions == "refresh") {
            config->refresh = true;
            config->cacheOptions = "";
        } else if (config->cacheOptions == "help") {
            Cli::subCommandUsage("cache");
            exit(0);
        } else if (config->cacheOptions == "report:missing=help") {
            Cli::cacheReportMissingUsage();
            exit(0);
        }
    }
    if (parser->isSet("startat")) {
        config->startAt = parser->value("startat");
    }
    if (parser->isSet("endat")) {
        config->endAt = parser->value("endat");
    }
    if (parser->isSet("includepattern")) {
        config->includePattern = parser->value("includepattern");
    }
    if (parser->isSet("excludepattern")) {
        config->excludePattern = parser->value("excludepattern");
    }
    if (parser->isSet("includefrom")) {
        config->includeFrom = parser->value("includefrom");
    }
    if (parser->isSet("excludefrom")) {
        config->excludeFrom = parser->value("excludefrom");
    }
    if (parser->isSet("maxfails") && parser->value("maxfails").toInt() >= 1 &&
        parser->value("maxfails").toInt() <= 200) {
        config->maxFails = parser->value("maxfails").toInt();
    }
    if (parser->isSet("region")) {
        config->region = parser->value("region");
    }
    if (parser->isSet("lang")) {
        config->lang = parser->value("lang");
    }
    if (parser->isSet("verbosity")) {
        config->verbosity = parser->value("verbosity").toInt();
    }
}

void RuntimeCfg::setFlag(const QString flag) {
    if (flag == "forcefilename") {
        config->forceFilename = true;
    } else if (flag == "interactive") {
        config->interactive = true;
    } else if (flag == "nobrackets") {
        config->brackets = false;
    } else if (flag == "nocovers") {
        config->cacheCovers = false;
    } else if (flag == "notextures") {
        config->cacheTextures = false;
    } else if (flag == "nocropblack") {
        config->cropBlack = false;
    } else if (flag == "nohints") {
        config->hints = false;
    } else if (flag == "nomarquees") {
        config->cacheMarquees = false;
    } else if (flag == "noresize") {
        config->cacheResize = false;
    } else if (flag == "noscreenshots") {
        config->cacheScreenshots = false;
    } else if (flag == "nosubdirs") {
        config->subdirs = false;
    } else if (flag == "nowheels") {
        config->cacheWheels = false;
    } else if (flag == "onlymissing") {
        config->onlyMissing = true;
    } else if (flag == "pretend") {
        config->pretend = true;
    } else if (flag == "relative") {
        config->relativePaths = true;
    } else if (flag == "skipexistingcovers") {
        config->skipExistingCovers = true;
    } else if (flag == "skipexistingmanuals") {
        config->skipExistingManuals = true;
    } else if (flag == "skipexistingmarquees") {
        config->skipExistingMarquees = true;
    } else if (flag == "skipexistingscreenshots") {
        config->skipExistingScreenshots = true;
    } else if (flag == "skipexistingvideos") {
        config->skipExistingVideos = true;
    } else if (flag == "skipexistingwheels") {
        config->skipExistingWheels = true;
    } else if (flag == "skipexistingtextures") {
        config->skipExistingTextures = true;
    } else if (flag == "skipped") {
        config->skipped = true;
    } else if (flag == "symlink") {
        config->symlink = true;
    } else if (flag == "theinfront") {
        config->theInFront = true;
    } else if (flag == "unattend") {
        config->unattend = true;
    } else if (flag == "unattendskip") {
        config->unattendSkip = true;
    } else if (flag == "unpack") {
        config->unpack = true;
    } else if (flag == "videos") {
        config->videos = true;
    } else if (flag == "manuals") {
        config->manuals = true;
    } else if (flag == "notidydesc") {
        config->tidyDesc = false;
    } else {
        printf("Unknown flag '%s', please check '--flags help' for "
               "a list of valid flags. Exiting...\n",
               flag.toStdString().c_str());
        exit(1);
    }
}

QSet<QString> RuntimeCfg::getKeys(CfgType type) {
    QSet<QString> ret;
    for (auto k : params.keys()) {
        int sections = params[k].second;
        if (type & sections) {
            ret.insert(k);
        }
    }
    return ret;
}

QString RuntimeCfg::concatPath(QString absPath, QString platformFolder) {
    if (absPath.right(1) != "/") {
        return absPath % "/" % platformFolder;
    }
    return absPath % platformFolder;
}

QStringList RuntimeCfg::parseFlags() {
    QStringList _flags{};
    if (parser->isSet("flags")) {
        QStringList flagsCli = parser->values("flags");
        for (QString f : flagsCli) {
            _flags << f.replace(" ", "").split(",");
        }
    }
    return _flags;
}

bool RuntimeCfg::validateFrontend(const QString &providedFrontend) {
    QStringList frontends = {"emulationstation", "retrobat", "attractmode",
                             "pegasus", "esde"};
    frontends.sort();
    if (!frontends.contains(providedFrontend)) {
        printf("\033[1;31mBummer! Unknown frontend '%s'. Known frontends are: "
               "%s.\033[0m\n",
               providedFrontend.toStdString().c_str(),
               frontends.join(", ").toStdString().c_str());
        return false;
    }
    return true;
}