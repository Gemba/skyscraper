/*
 *  This file is part of skyscraper.
 *  Copyright 2017 Lars Muldjord
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

#include "cli.h"

#include "cache.h"
#include "config.h"
#include "strtools.h"

#include <QCommandLineOption>
#include <QDomDocument>
#include <QMapIterator>
#include <QStringBuilder>

#if QT_VERSION >= 0x050a00
#include <QRandomGenerator>
#endif

void Cli::createParser(QCommandLineParser *parser, QString platforms) {

    QString h =
        "Skyscraper looks for compatible game files for the chosen platform "
        "(set with '-p'). It allows you to gather and cache media and game "
        "information for the files using various scraping modules (set with "
        "'-s'). It then lets you generate game lists for the supported "
        "frontends by combining all previously cached resources ('game list "
        "generation mode' is initiated by simply leaving out the '-s' option). "
        "While doing so it also composites game art for all files by following "
        "the recipe at '" %
        Config::getSkyFolder() %
        "/artwork.xml'.\n\nIn addition "
        "to the command line options Skyscraper also provides a lot of "
        "customizable options for configuration, artwork, game name aliases, "
        "resource priorities and much more. Please check the full "
        "documentation at 'https://gemba.github.io/skyscraper/' for a "
        "detailed explanation of all features.\n\nRemember that most of the "
        "following options can also be set in the "
        "'" %
        Config::getSkyFolder() %
        "/config.ini' file. All command line options "
        "and config.ini options are thoroughly documented at the above link.";

    QString ht = "";
    int csr = 0;
    for (auto w : h.split(QRegExp("\\s"))) {
        if (csr + w.length() >= 80 || w.isEmpty()) {
            csr = 0;
            ht.append('\n');
            if (w.isEmpty()) {
                ht.append("\n");
                continue;
            }
        }
        csr += w.length() + 1;
        ht.append(w);
        ht.append(' ');
    }
    ht.chop(1);

    parser->setApplicationDescription(StrTools::getVersionHeader() + ht);

    QCommandLineOption pOption(
        "p",
        "The platform you wish to scrape. Currently supports " + platforms +
            ".",
        "PLATFORM", "");
    QCommandLineOption fOption(
        "f",
        "The frontend you wish to generate a gamelist for. Remember to leave "
        "out the '-s' option when using this in order to enable Skyscraper's "
        "gamelist generation mode.\nCurrently supports 'emulationstation', "
        "'esde', 'retrobat', 'attractmode' and 'pegasus'. Default: "
        "'emulationstation'",
        "FRONTEND", "");
    QCommandLineOption eOption(
        "e",
        "Set extra frontend option. This is required by the 'attractmode' "
        "frontend to set the emulator and optionally for the 'pegasus' "
        "frontend to set the launch command. Default: unset",
        "STRING", "");
    QCommandLineOption iOption(
        "i",
        "Folder which contains the game/rom files. Default: "
        "'" %
            QDir::homePath() % "/RetroPie/roms/PLATFORM'",
        "PATH", "");
    QCommandLineOption gOption(
        "g", "Game list export folder.\n(default depends on frontend)", "PATH",
        "");
    QCommandLineOption oOption(
        "o", "Game media export folder.\n(default depends on frontend)", "PATH",
        "");
    QCommandLineOption sOption(
        "s",
        "The scraping module you wish to gather resources from for the "
        "platform set with '-p'.\nLeave the '-s' option out to enable "
        "Skyscraper's gamelist generation mode.\n(WEB: 'arcadedb', 'igdb', "
        "'mobygames', 'openretro', 'screenscraper', 'thegamesdb' and "
        "'worldofspectrum', LOCAL: 'esgamelist' and 'import')",
        "MODULE", "");
    QCommandLineOption uOption(
        "u",
        "userKey or UserID and Password for use with the selected scraping "
        "module. Default: unset)",
        "KEY/USER:PASSWORD", "");
    QCommandLineOption mOption(
        "m",
        "Minimum match percentage when comparing search result titles to "
        "filename titles. Default: 65",
        "0-100", "");
    QCommandLineOption lOption(
        "l",
        "Maximum game description length. Everything longer than this will be "
        "truncated. Default: 2500",
        "0-10000", "");
    QCommandLineOption tOption(
        "t",
        "Number of scraper threads to use. This might change depending on the "
        "scraping module limits. Default is 4",
        "1-8", "");
    QCommandLineOption cOption(
        "c",
        "Use this config file to set up Skyscraper. Default: "
        "'" %
            Config::getSkyFolder() % "/config.ini'",
        "FILENAME", "");
    QCommandLineOption aOption(
        "a",
        "Specify a non-default artwork.xml file to use when setting up the "
        "artwork compositing when in gamelist generation mode. Default: "
        "'" %
            Config::getSkyFolder() % "/artwork.xml'",
        "FILENAME", "");
    QCommandLineOption dOption(
        "d",
        "Set custom resource cache folder. Default: "
        "'" %
            Config::getSkyFolder(Config::SkyFolderType::CACHE) % "/PLATFORM'",
        "FOLDER", "");
    QCommandLineOption addextOption(
        "addext",
        "Add this or these file extension(s) to accepted file extensions "
        "during a scraping run. (example: '*.zst' or '*.zst *.ext')",
        "EXTENSION(S)", "");
    QCommandLineOption flagsOption(
        "flags",
        "Allows setting flags that will impact the run in various ways. See "
        "'--flags help' for a list of all available flags and what they do.",
        "FLAG1,FLAG2,...", "");
    QCommandLineOption cacheOption(
        "cache",
        "This option is the master option for all options related to the "
        "resource cache. It must be followed by 'COMMAND[:OPTIONS]'.\nSee "
        "'--cache help' for a full description of all functions.",
        "COMMAND[:OPTIONS]", "");
    QCommandLineOption gamelistfilenameOption(
        "gamelistfilename",
        "Game list export filename.\n(default depends on frontend)", "FILENAME",
        "");
    QCommandLineOption refreshOption("refresh", "Same as '--cache refresh'.");
    QCommandLineOption startatOption(
        "startat",
        "Tells Skyscraper which file to start at. Forces '--refresh' and "
        "'--flags nosubdirs' enabled.",
        "FILENAME", "");
    QCommandLineOption endatOption(
        "endat",
        "Tells Skyscraper which file to end at. Forces '--refresh' and "
        "'--flags nosubdirs' enabled.",
        "FILENAME", "");
    QCommandLineOption includepatternOption(
        "includepattern",
        "Tells Skyscraper to only include the files matching the provided "
        "asterisk pattern(s). Remember to double-quote the pattern to avoid "
        "weird behaviour. You can add several patterns by separating them with "
        "','. In cases where you need to match for a comma you need to escape "
        "it as '\\,'. (Pattern example: '\"Super*,*Fighter*\"')",
        "PATTERN", "");
    QCommandLineOption excludepatternOption(
        "excludepattern",
        "Tells Skyscraper to always exclude the files matching the provided "
        "asterisk pattern(s). Remember to double-quote the pattern to avoid "
        "weird behaviour. You can add several patterns by separating them with "
        "','. In cases where you need to match for a comma you need to escape "
        "it as '\\,'. (Pattern example: '\"*[BIOS]*,*proto*\"')",
        "PATTERN", "");
    QCommandLineOption includefromOption(
        "includefrom",
        "Tells Skyscraper to only include the files listed in FILENAME. One "
        "filename per line. This file can be generated with the '--cache "
        "report:missing' option or made manually.",
        "FILENAME", "");
    QCommandLineOption excludefromOption(
        "excludefrom",
        "Tells Skyscraper to exclude all files listed in FILENAME. One "
        "filename per line. This file can be generated with the '--cache "
        "report:missing' option or made manually.",
        "FILENAME", "");
    QCommandLineOption maxfailsOption(
        "maxfails",
        "Sets the allowed number of initial 'Not found' results before "
        "rage-quitting. Default: 42",
        "1-200", "");
    QCommandLineOption queryOption(
        "query",
        "Allows you to set a custom search query (eg. 'rick+dangerous' for "
        "name based modules or 'sha1=CHECKSUM', 'md5=CHECKSUM' or "
        "'romnom=FILENAME' for the 'screenscraper' module). Requires the "
        "single rom filename you wish to override for to be passed on command "
        "line as well, otherwise it will be ignored.",
        "QUERY", "");
    QCommandLineOption regionOption(
        "region",
        "Add preferred game region for scraping modules that support "
        "it. Default prioritization: 'eu', 'us', 'wor' and 'jp' + others "
        "in that order.",
        "CODE", "eu");
    QCommandLineOption langOption(
        "lang",
        "Set preferred result language for scraping modules that support it. "
        "Default: 'en'",
        "CODE", "en");
    QCommandLineOption verbosityOption(
        "verbosity", "Print more info while scraping. Default: 0", "0-3", "0");
    QCommandLineOption hintOption("hint",
                                  "Show a random 'Tip of the Day' and quit.");

    parser->addOption(addextOption);
    parser->addOption(aOption);
    parser->addOption(cacheOption);
    parser->addOption(cOption);
    parser->addOption(dOption);
    parser->addOption(endatOption);
    parser->addOption(eOption);
    parser->addOption(excludefromOption);
    parser->addOption(excludepatternOption);
    parser->addOption(flagsOption);
    parser->addOption(fOption);
    parser->addOption(gOption);
    parser->addOption(gamelistfilenameOption);
    parser->addHelpOption();
    parser->addOption(hintOption);
    parser->addOption(includefromOption);
    parser->addOption(includepatternOption);
    parser->addOption(iOption);
    parser->addOption(langOption);
    parser->addOption(lOption);
    parser->addOption(maxfailsOption);
    parser->addOption(mOption);
    parser->addOption(oOption);
    parser->addOption(pOption);
    parser->addOption(queryOption);
    parser->addOption(refreshOption);
    parser->addOption(regionOption);
    parser->addOption(sOption);
    parser->addOption(startatOption);
    parser->addOption(tOption);
    parser->addOption(uOption);
    parser->addOption(verbosityOption);
    parser->addVersionOption();
    parser->addPositionalArgument(
        "romfile", "Specific ROM to scrape, optionally.", "[<romfile>]");
}

void Cli::subCommandUsage(const QString subCmd) {

    if (subCmd == "flags") {
        printf("Use comma-separated flags (eg. '--flags FLAG1,FLAG2') or use "
               "--flags option\nmultiple times to enable multiple flags. The "
               "following is a list of valid flags\nand what they do.\n");
    }

    printf("\nShowing '\033[1;33m--%s ...\033[0m' help:\n\n",
           subCmd.toUtf8().constData());

    QMap<QString, QString> subOptions;
    subOptions = getSubCommandOpts(subCmd);

    int keywordMaxLen = 0;
    for (auto k : subOptions.keys()) {
        if (k.length() > keywordMaxLen) {
            keywordMaxLen = k.length();
        }
    }
    int textWidth = 80 - 2 - 2 - keywordMaxLen;

    QMapIterator<QString, QString> i(subOptions);
    QString lb("\n");
    while (i.hasNext()) {
        i.next();
        QString k = i.key().leftJustified(keywordMaxLen, ' ');
        QString v;
        int ptr = 0;
        for (auto w : i.value().split(' ')) {
            if (ptr + w.length() >= textWidth) {
                ptr = 0;
                v.append(lb.leftJustified(keywordMaxLen + 5, ' '));
            }
            ptr += w.length() + 1;
            v.append(w);
            v.append(' ');
        }
        v.chop(1);
        printf("\033[1;33m  %s\033[0m  %s\n", k.toUtf8().constData(),
               v.toUtf8().constData());
    }
    printf("\n");
}

QMap<QString, QString> Cli::getSubCommandOpts(const QString subCmd) {
    QMap<QString, QString> m;
    if (subCmd == "cache") {
        m = {
            {"help", "Prints this help and exits."},
            {"show", "Prints a status of all cached resources for the selected "
                     "platform."},
            {"validate",
             "Checks the consistency of the cache for the selected platform."},
            {"edit",
             "Let's you edit resources for the selected platform for all files "
             "or a range of files. Add a filename on command line to edit "
             "cached resources for just that one file, use '--includefrom' to "
             "edit files created with the '--cache report' option or use "
             "'--startat' and '--endat' to edit a range of roms."},
            {"edit:new=<TYPE>",
             "Let's you batch add resources of <TYPE> to the selected platform "
             "for all files or a range of files. Add a filename on command "
             "line to edit cached resources for just that one file, use "
             "'--includefrom' to edit files created with the '--cache report' "
             "option or use '--startat' and '--endat' to edit a range of "
             "roms."},
            {"vacuum",
             "Compares your romset to any cached resource and removes the "
             "resources that you no longer have roms for."},
            {"report:missing=<OPTION>",
             "Generates reports with all files that are missing the "
             "specified resources. Check '--cache report:missing=help' "
             "for more info."},
            {"merge:<PATH>",
             "Merges two resource caches together. It will merge the resource "
             "cache specified by <PATH> into the local resource cache by "
             "default. To merge into a non-default destination cache folder "
             "set it with '-d <PATH>'. Both should point to folders with the "
             "'db.xml' inside."},
            {"purge:all",
             "Removes ALL cached resources for the selected platform."},
            {"purge:m=<MODULE>,t=<TYPE>",
             "Removes cached resources related to the selected module(m) and / "
             "or type(t). Either one can be left out in which case ALL "
             "resources from the selected module or ALL resources rom the "
             "selected type will be removed."},
            {"refresh", "Forces a refresh of existing cached resources for any "
                        "scraping module. Requires a scraping module set with "
                        "'-s'. Similar to '--refresh'."},
        };
    } else if (subCmd == "flags") {
        m = {
            {"help", "Prints this help and exits."},
            {"forcefilename",
             "Use filename as game name instead of the returned game title "
             "when generating a game list. Consider using 'nameTemplate' "
             "config.ini option instead."},
            {"interactive", "Always ask user to choose best returned result "
                            "from the scraping modules."},
            {"nobrackets",
             "Disables any [] and () tags in the frontend game titles. "
             "Consider using 'nameTemplate' config.ini option instead."},
            {"nocovers",
             "Disable covers/boxart from being cached locally. Only do this if "
             "you do not plan to use the cover artwork in 'artwork.xml'"},
            {"nocropblack",
             "Disables cropping away black borders around screenshot resources "
             "when compositing the final gamelist artwork."},
            {"nohints",
             "Disables the 'DID YOU KNOW:' hints when running Skyscraper."},
            {"nomarquees",
             "Disable marquees from being cached locally. Only do this if you "
             "do not plan to use the marquee artwork in 'artwork.xml'"},
            {"noresize",
             "Disable resizing of artwork when saving it to the resource "
             "cache. Normally they are resized to save space."},
            {"noscreenshots", "Disable screenshots/snaps from being cached "
                              "locally. Only do this if you do not plan to use "
                              "the screenshot artwork in 'artwork.xml'"},
            {"nosubdirs",
             "Do not include input folder subdirectories when scraping."},
            {"notidydesc", "Disables tidying common misformats in description "
                           "text. See manual CLIHELP.md for details."},
            {"nowheels",
             "Disable wheels from being cached locally. Only do this if you do "
             "not plan to use the wheel artwork in 'artwork.xml'"},
            {"onlymissing", "Tells Skyscraper to skip all files which already "
                            "have any data from any source in the cache."},
            {"pretend",
             "Only relevant when generating a game list. It disables the game "
             "list generator and artwork compositor and only outputs the "
             "results of the potential game list generation to the terminal. "
             "Use it to check what and how the data will be combined from "
             "cached resources."},
            {"relative",
             "Forces all gamelist paths to be relative to rom location."},
            {"skipexistingcovers",
             "When generating gamelists, skip processing covers that already "
             "exist in the media output folder."},
            {"skipexistingmanuals",
             "When generating gamelists, skip processing manuals that already "
             "exist in the media output folder."},
            {"skipexistingmarquees",
             "When generating gamelists, skip processing marquees that already "
             "exist in the media output folder."},
            {"skipexistingscreenshots",
             "When generating gamelists, skip processing screenshots that "
             "already exist in the media output folder."},
            {"skipexistingtextures",
             "When generating gamelists, skip processing textures, covers, "
             "disc art that already exist in the media output folder."},
            {"skipexistingvideos",
             "When generating gamelists, skip copying videos that already "
             "exist in the media output folder."},
            {"skipexistingwheels",
             "When generating gamelists, skip processing wheels that already "
             "exist in the media output folder."},
            {"skipped", "When generating a gamelist, also include games that "
                        "do not have any cached data."},
            {"symlink",
             "Forces cached videos to be symlinked to game list destination to "
             "save space. WARNING! Deleting or moving files from your cache "
             "can invalidate the links!"},
            {"theinfront",
             "Forces Skyscraper to always try and move 'The' to the beginning "
             "of the game title when generating gamelists. By default 'The' "
             "will be moved to the end of the game titles."},
            {"unattend",
             "Skip initial questions when scraping. It will then always "
             "overwrite existing gamelist and not skip existing entries."},
            {"unattendskip",
             "Skip initial questions when scraping. It will then always "
             "overwrite existing gamelist and always skip existing entries."},
            {"unpack",
             "Unpacks and checksums the file inside 7z or zip files instead of "
             "the compressed file itself. Be aware that this option requires "
             "'7z' to be installed on the system to work. Only relevant for "
             "'screenscraper' scraping module."},
            {"videos",
             "Enables scraping and caching of videos for the scraping modules "
             "that support them. Beware, this takes up a lot of disk space!"},
            {"manuals",
             "Enables scraping and caching of manuals for the scraping modules "
             "that support them."},
        };
    } else {
        QStringList resTypes = Cache::getAllResourceTypes();
        resTypes.sort();
        m = {
            {"help", "Shows this help message and exits."},
            {"all", "Creates reports for all resource types."},
            {"textual", "Creates reports for all textual resource types."},
            {"artwork", "Creates reports for all artwork related resource "
                        "types excluding 'video' and 'manual'."},
            {"media", "Creates reports for all media resource types including "
                      "'video' and 'manual'."},
            {"type1,type2,type3,...",
             "Creates reports for selected types. Example: "
             "'developer,screenshot,rating'. Available resource types: " +
                 resTypes.join(", ")},
        };
    }
    return m;
}

void Cli::cacheReportMissingUsage() {
    subCommandUsage("cache report:missing=");
}

void Cli::showHint() {
    QFile hintsFile("hints.xml");
    QDomDocument hintsXml;
    if (!hintsFile.open(QIODevice::ReadOnly) ||
        !hintsXml.setContent(&hintsFile)) {
        return;
    }
    hintsFile.close();
    QDomNodeList hintNodes = hintsXml.elementsByTagName("hint");
    printf("\033[1;33mDID YOU KNOW:\033[0m %s\n\n",
           hintsXml
               .elementsByTagName("hint")
#if QT_VERSION >= 0x050a00
               .at(QRandomGenerator::global()->generate() % hintNodes.length())
#else
               .at(qrand() % hintNodes.length())
#endif
               .toElement()
               .text()
               .toStdString()
               .c_str());
}
