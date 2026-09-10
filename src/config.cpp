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

#include "config.h"

#include "nocolor.h"
#include "pathtools.h"
#include "platform.h"
#include "skyscraper.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStringBuilder>
#include <QStringList>
#include <QTextStream>
#include <filesystem>

Config::SkyFolders skyFolders;

void Config::initSkyFolders() {
    const QString appFolder = "skyscraper";
#ifndef XDG
    // genuine Skyscraper folders
    skyFolders[SkyFolderType::CONFIG] = QDir::homePath() % "/." % appFolder;
    skyFolders[SkyFolderType::CACHE] =
        skyFolders[SkyFolderType::CONFIG] % "/cache";
    skyFolders[SkyFolderType::IMPORT] =
        skyFolders[SkyFolderType::CONFIG] % "/import";
    skyFolders[SkyFolderType::RESOURCE] =
        skyFolders[SkyFolderType::CONFIG] % "/resources";
    skyFolders[SkyFolderType::REPORT] =
        skyFolders[SkyFolderType::CONFIG] % "/reports";
    skyFolders[SkyFolderType::LOG] = skyFolders[SkyFolderType::CONFIG];
#else
    // XDG Spec
    QMap<QString, QString> xdgEnvs = {
        {"XDG_CONFIG_HOME", QDir::homePath() % "/.config"},
        {"XDG_CACHE_HOME", QDir::homePath() % "/.cache"},
        {"XDG_DATA_HOME", QDir::homePath() % "/.local/share"},
        {"XDG_STATE_HOME", QDir::homePath() % "/.local/state"}};

    for (const auto &e : xdgEnvs.keys()) {
        QString xdgDir =
            QProcessEnvironment::systemEnvironment().value(e, xdgEnvs.value(e));
        if (QFileInfo(xdgDir).isAbsolute()) {
            xdgEnvs[e] = xdgDir;
            QDir d(xdgDir % "/" % appFolder);
            if (!d.exists() && !d.mkpath(".")) {
                ncprintf("Cannot create folder '%s'. Please check permissions, "
                         "now exiting...\n",
                         d.absolutePath().toStdString().c_str());
                emit die(1,
                         QString("cannot create directory '%1'")
                             .arg(d.absolutePath()),
                         "Permission denied");
            }
        }
    }

    skyFolders[SkyFolderType::CONFIG] =
        xdgEnvs["XDG_CONFIG_HOME"] % "/" % appFolder;
    skyFolders[SkyFolderType::CACHE] =
        xdgEnvs["XDG_CACHE_HOME"] % "/" % appFolder;
    skyFolders[SkyFolderType::IMPORT] =
        xdgEnvs["XDG_DATA_HOME"] % "/" % appFolder % "/import";
    skyFolders[SkyFolderType::RESOURCE] =
        xdgEnvs["XDG_DATA_HOME"] % "/" % appFolder % "/resources";
    skyFolders[SkyFolderType::REPORT] =
        xdgEnvs["XDG_STATE_HOME"] % "/" % appFolder % "/reports";
    skyFolders[SkyFolderType::LOG] =
        xdgEnvs["XDG_STATE_HOME"] % "/" % appFolder;

    QDir(skyFolders[SkyFolderType::IMPORT]).mkpath(".");
    QDir(skyFolders[SkyFolderType::RESOURCE]).mkpath(".");
    QDir(skyFolders[SkyFolderType::REPORT]).mkpath(".");
#endif // XDG

#ifndef QT_NO_DEBUG_OUTPUT
    qDebug("Skyscraper folder config:");
    for (auto it : skyFolders.toStdMap()) {
        qDebug() << static_cast<int>(it.first) << it.second;
    }
#endif
}

QString Config::getSkyFolder(SkyFolderType type) { return skyFolders[type]; }

void Config::copyFile(const QString &src, const QString &dest, bool isPristine,
                      FileOp fileOp) {
    if (!QFileInfo::exists(src)) {
        ncprintf("\033[1;31mSource config file not found '%s'. Please check "
                 "setup, bailing out...\033[0m\n",
                 src.toStdString().c_str());
        emit die(1, QString("cannot access '%1'").arg(src),
                 "File does not exist");
    }

    if (QFileInfo::exists(dest)) {
        if (fileOp == FileOp::OVERWRITE) {
            if (isPristine && (src.endsWith("peas.json") ||
                               src.endsWith("platforms_idmap.csv"))) {
                // remove possible destination *.dist file
                if (QFile::remove(dest % ".dist")) {
                    qDebug() << (dest % ".dist") << "removed as prisitine"
                             << dest << "detected";
                }
            }
            if (QFileInfo(src).size() != QFileInfo(dest).size()) {
                QFile::remove(dest);
                QFile::copy(src, dest);
                qDebug() << "Overwritten file" << dest
                         << "bc. file size differed";
            }
        } else if (fileOp == FileOp::CREATE_DIST) {
            QString distfn = QString(dest + ".dist");
            if (QFileInfo(src).size() != QFileInfo(dest).size()) {
                QFile::remove(distfn);
                QFile::copy(src, distfn);
                qDebug() << "Copied original distribution file" << src << "as"
                         << distfn;
            }
        }
    } else {
        QFile::copy(src, dest);
        qDebug() << "Created file" << dest;
    }
}

Config::Setup Config::setupUserConfig() {
    QDir skyDir(getSkyFolder());
    if (!skyDir.exists()) {
        if (!skyDir.mkpath(".")) {
            ncprintf("Cannot create folder '%s'. Please check permissions, "
                     "now exiting...\n",
                     skyDir.absolutePath().toStdString().c_str());
            emit die(1,
                     QString("cannot create directory '%1'")
                         .arg(skyDir.absolutePath()),
                     "Permission denied");
        }
    }

    // Set the working directory to the applications config path
    // defaults to the folder containing config.ini, artwork.xml, hints.xml, ...
    // any file outside this folder or subfolders to this folder shall use
    // Config::getSkyFolder(type ...)
    QDir::setCurrent(skyDir.absolutePath());

    // Create import paths
    QStringList paths = {"covers",  "manuals",  "marquees", "screenshots",
                         "textual", "textures", "videos",   "wheels"};
    for (auto p : paths) {
        QDir(getSkyFolder(SkyFolderType::IMPORT) % "/" % p).mkpath(".");
    }

    // Create resources folder
    QDir(getSkyFolder(SkyFolderType::RESOURCE)).mkpath(".");

    // Create cache folder
    QDir(getSkyFolder(SkyFolderType::CACHE)).mkpath(".");

    QString rpInst = "/opt/retropie/supplementary/skyscraper/Skyscraper";
    bool isRpInstall = QFileInfo(rpInst).isFile();
    QString manualInst = PREFIX "/bin/Skyscraper";
    bool exeIsNotSymlink =
        QFileInfo(manualInst).isFile() && !QFileInfo(manualInst).isSymLink();

    if (exeIsNotSymlink && isRpInstall) {
        ncprintf("\033[1;31mDuplicate installation of Skyscraper found:\n%s "
                 "and\n%s\nPlease remove one or the other to avoid "
                 "confusion.\033[0m\n",
                 manualInst.toStdString().c_str(),
                 rpInst.toStdString().c_str());
        emit die(1, "duplicate installation files detected",
                 "Confused Skyscraper problem");
    }

    if (isRpInstall) {
        QString tgtDir = getSkyFolder();
        for (auto const &src :
             QStringList({"peas.json", "platforms_idmap.csv"})) {
            // just issue the warning if needed, file copy is done with
            // RetroPie's scriptmodule
            isPlatformCfgPristine(tgtDir % "/" % src);
        }
        // no need to install/check config files (done via scriptmodule).
        return Setup::RETROPIE;
    }

#ifdef Q_OS_WIN
    // config deploymend done externally
    return Setup::WINDOWS;
#endif

    QString prefixBinPath = QString(PREFIX "/bin/");
    QString sysConfPath = QString(SYSCONFDIR);
    QString applPath = QCoreApplication::applicationDirPath();
    QString configSourcePath =
        validateConfigSrcPath(sysConfPath, prefixBinPath, applPath);

    if (configSourcePath.isEmpty()) {
        // manual deployment of Skyscraper binary
        // essential configs will be loaded from Qt-resource cache of Skyscraper
        return Setup::SINGLE_BIN;
    }

    int isPristine;
    for (const auto &src : configFiles.keys()) {
        QString dest = configFiles.value(src).first;
        isPristine = false;
        if (dest.isEmpty()) {
            dest = QString(src).replace("!", "");
        }
        QString tgtDir = adjustDestinationPath(src, dest);
        if ((src == "peas.json" || src == "platforms_idmap.csv")) {
            isPristine = isPlatformCfgPristine(tgtDir % "/" % dest);
            // isPristine == 1: keep updated files from release in *.dist
            if (isPristine == 0) {
                configFiles[src].second = FileOp::OVERWRITE;
            } else if (isPristine < 0) {
                ncprintf("\033[1;31mFile '%s' cannot be read. "
                         "Please fix. Quitting.\033[0m\n",
                         (tgtDir % "/" % dest).toUtf8().constData());
                emit die(1,
                         QString("cannot access '%1'").arg(tgtDir % "/" % dest),
                         "Permission denied");
            }
        }
        QString tgt = tgtDir % "/" % dest;
        copyFile(configSourcePath % "/" % QString(src).replace("!", ""), tgt,
                 isPristine == 0, configFiles.value(src).second);
    }
    return Setup::MAKE;
}

bool Config::createDefaultConfigIni() {
    const QString tgtFn = PathTools::concatPath(getSkyFolder(), "config.ini");
    if (QFile(tgtFn).exists()) {
        return true;
    }
    QString srcFn = PathTools::locateConfigFile("config.ini");
    if (QFile::copy(srcFn, tgtFn)) {
        if (srcFn.startsWith(":/")) {
            QFile::setPermissions(
                tgtFn, QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                           QFileDevice::ReadGroup | QFileDevice::ReadOther);
        }
        ncprintf("Successfully created default config in "
                 "'\033[1m%s\033[0m'.\nHappy scraping!\n\n",
                 PathTools::pathToStdStr(tgtFn).c_str());
        return true;
    }
    ncprintf("\033[1;31mCannot create config at '%s'. Check file path and "
             "permissions!\033[0m\n",
             PathTools::pathToStdStr(tgtFn).c_str());
    return false;
}

void Config::configInfo() {
    ncprintf("\033[1;34mMinimum set of configuration files to get Skyscraper "
             "running.\033[0m\n");
    ncprintf("\033[1m%-28s     %s\033[0m\n", "File:", "Source:");
    bool last;
    for (const auto &k : essentialCfgs.keys()) {
        QString dest = k;
        QString tgtDir = adjustDestinationPath(k, dest);
        QString tgt = tgtDir % "/" % dest;
        if (tgt.isEmpty()) {
            qWarning() << k;
        }
        last = k == "tgdb_publishers.json";
        ncprintf("%s── \033[1m%-28s  %s\033[0m\n", last ? "└" : "├",
                 k.toStdString().c_str(),
                 QFile(tgt).exists() ? PathTools::pathToStdStr(tgt).c_str()
                                     : "built-in");
        ncprintf("%s   └── %s\n", last ? " " : "│",
                 essentialCfgs[k].toStdString().c_str());
    }
    const QString cfgFn = PathTools::concatPath(getSkyFolder(), "config.ini");
    if (!QFile(cfgFn).exists()) {
        ncprintf(
            "\n\033[1;33mWarning\033[0m: There is no default configuration "
            "file '\033[1mconfig.ini\033[0m' present in\n  '%s'\nSkyscraper "
            "can be run without, but it is recommended to use such "
            "file.\nExecute 'Skyscraper --ini' to create a config file with "
            "the defaults.\n",
            PathTools::pathToStdStr(Config::getSkyFolder()).c_str());
    }
    ncprintf("\nExpected folders for these files, if you want to override "
             "built-ins:\n");
    ncprintf("%-12s: \033[1;32m%s\033[0m\n", "Main configs",
             PathTools::pathToStdStr(getSkyFolder()).c_str());
    ncprintf(
        "%-12s: \033[1;32m%s\033[0m\n", "Cache",
        PathTools::pathToStdStr(getSkyFolder(SkyFolderType::CACHE)).c_str());
    ncprintf(
        "%-12s: \033[1;32m%s\033[0m\n", "Import",
        PathTools::pathToStdStr(getSkyFolder(SkyFolderType::IMPORT)).c_str());
    ncprintf(
        "%-12s: \033[1;32m%s\033[0m\n", "Resources",
        PathTools::pathToStdStr(getSkyFolder(SkyFolderType::RESOURCE)).c_str());
    ncprintf("\033[1;34mFor details on these files and usage do consult the "
             "documentation.\033[0m\n");
}

QString Config::adjustDestinationPath(const QString &src, QString &dest) {
    QString tgtDir = getSkyFolder(SkyFolderType::CONFIG);
    if (src.startsWith("cache/") || src == "CACHE.md") {
        tgtDir = getSkyFolder(SkyFolderType::CACHE);
        dest = dest.replace("cache/", "");
    } else if (src.startsWith("import/")) {
        tgtDir = getSkyFolder(SkyFolderType::IMPORT);
        dest = dest.replace("import/", "");
    } else if (src.startsWith("resources/")) {
        tgtDir = getSkyFolder(SkyFolderType::RESOURCE);
        dest = dest.replace("resources/", "");
    }
    return tgtDir;
}

QString Config::validateConfigSrcPath(const QString &sysConfPath,
                                      const QString &prefixPath,
                                      const QString &applDirPath) {
    QString configSourcePath = PathTools::concatPath(sysConfPath, "skyscraper");
    const QString prefixBinPath = PathTools::concatPath(prefixPath, "bin");
    qDebug() << "configSourcePath initial" << configSourcePath;
    if (!QFileInfo(configSourcePath).exists()) {
        if (!applDirPath.startsWith(prefixBinPath, Qt::CaseInsensitive)) {
            // could be AppImage or expanded AppImage
            QString tmpPiggy =
                QString(applDirPath).replace(prefixBinPath, configSourcePath);
            configSourcePath = tmpPiggy;
        }
    }
    if (configSourcePath.startsWith(applDirPath) /* PR #261 */
        || !QFileInfo(configSourcePath).exists()) {
        if (!QFileInfo(configSourcePath).exists())
            qDebug() << "config source path does not exist" << configSourcePath;
        return "";
    }
    qDebug() << "config source path is" << configSourcePath;
    return configSourcePath;
}

int Config::isPlatformCfgPristine(QString platformCfgFilePath) {

    int isPristine =
        Platform::get().isPlatformCfgfilePristine(platformCfgFilePath);
    if (isPristine == 1) {
        ncprintf("\033[1;33mLooks like '%s' has local changes.\nPlease "
                 "transfer local changes to another file to mute this "
                 "warning.\nSee topic 'Transferring Local Platform Changes' "
                 "in the PLATFORM.md documentation for guidance.\033[0m\n",
                 platformCfgFilePath.toUtf8().constData());
    }

    return isPristine;
}

void Config::checkLegacyFiles() {
    QStringList legacyJsons = {"mobygames", "platforms", "screenscraper"};
    for (auto bn : legacyJsons) {
        QString fn = getSkyFolder() % "/" % bn % ".json";
        if (QFileInfo::exists(fn)) {
            ncprintf(
                "\033[1;33mFile '%s' found, which is no longer used in this "
                "version of Skyscraper. Please move file to mute this warning. "
                "See docs/PLATFORMS.md for additional info.\033[0m\n",
                fn.toUtf8().constData());
        }
    }
}

QString Config::getSupportedPlatforms() {
    if (!Platform::get().loadConfig()) {
        emit die(1, "cannot parse platform configuration",
                 "Platform configuration files (peas*.json / "
                 "platforms_idmap*.csv) missing or erroneous");
    }

    QString platforms;
    for (const auto &platform : Platform::get().getPlatforms()) {
        platforms.append("'" % platform % "', ");
    }
    platforms.chop(2);
    return platforms;
}

QString Config::getRetropieVersion() {
    // return RetroPie version if any
    QString ver = "";
    QFile rpVer("/opt/retropie/VERSION");
    if (rpVer.open(QIODevice::ReadOnly)) {
        QTextStream in(&rpVer);
        while (!in.atEnd()) {
            ver = in.readLine();
            break;
        }
        rpVer.close();
    }
    return ver;
}