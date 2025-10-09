/***************************************************************************
 *            platform.cpp
 *
 *  Sat Dec 23 10:00:00 CEST 2017
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

#include "platform.h"

#include "config.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVersionNumber>

static const QString fnPeas = "peas.json";
static const QString fnPeasLocal = "peas_local.json";
static const QString fnPlatformsIdMap = "platforms_idmap.csv";
static const QString fnPlatformsIdMapLocal = "platforms_idmap_local.csv";

Platform &Platform::get() {
    static Platform platform;
    return platform;
}

QStringList Platform::getPlatforms() const { return platforms; }

// If user provides no scraping source with '-s' this sets the default for the
// platform
QString Platform::getDefaultScraper() const { return "cache"; }

bool Platform::loadConfig() {
    clearConfigData();

    QFile configFile(fnPeas);
    if (!configFile.open(QIODevice::ReadOnly)) {
        QString extraInfo = "";
        if (QString rpVer = Config::getRetropieVersion(); !rpVer.isEmpty()) {
            QVersionNumber foundVer = QVersionNumber::fromString(rpVer);
            QVersionNumber reqVer = QVersionNumber::fromString("4.8.6");
            if (QVersionNumber::compare(foundVer, reqVer) == -1) {
                extraInfo =
                    QString(
                        "\n\nIt seems you are using Skyscraper in a RetroPie "
                        "setup. The identified RetroPie\nversion is %1, "
                        "the missing file was introduced with version %2: "
                        "Update\nyour RetroPie-Setup script and re-install "
                        "Skyscraper via retropie_setup.sh to\nremediate this "
                        "error.")
                        .arg(foundVer.toString())
                        .arg(reqVer.toString());
            }
        }
        printf("\033[1;31mFile not found '%s'.%s Now quitting...\033[0m\n",
               fnPeas.toUtf8().constData(), extraInfo.toUtf8().constData());
        return false;
    }

    QByteArray jsonData = configFile.readAll();
    QJsonDocument json(QJsonDocument::fromJson(jsonData));

    if (json.isNull() || json.isEmpty()) {
        printf("\033[1;31mFile '%s' empty or no JSON format. Now "
               "quitting...\033[0m\n",
               fnPeas.toUtf8().constData());
        return false;
    }

    QJsonObject jObjLocal = loadLocalConfig();
    QJsonObject jObj = json.object();

    for (auto plit = jObjLocal.constBegin(); plit != jObjLocal.constEnd();
         plit++) {
        jObj.insert(plit.key(), plit.value());
    }

    peas = jObj.toVariantMap();

    for (auto piter = jObj.constBegin(); piter != jObj.constEnd(); piter++) {
        platforms.push_back(piter.key());
    }

    platforms.sort();

    if (!loadPlatformsIdMap()) {
        return false;
    }
    return true;
}

QJsonObject Platform::loadLocalConfig() {
    QJsonObject peasLocal;
    QFile configFile(fnPeasLocal);

    if (configFile.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = configFile.readAll();
        QJsonDocument json(QJsonDocument::fromJson(jsonData));

        if (!json.isNull() && !json.isEmpty()) {
            qDebug() << "successfully loaded" << fnPeasLocal;
            peasLocal = json.object();
        }
    }
    return peasLocal;
}

void Platform::clearConfigData() {
    platforms.clear();
    platformIdsMap.clear();
}

QString Platform::getFormats(QString platform, QString extensions,
                             QString addExtensions) const {
    if (!extensions.isEmpty() && extensions.contains("*.")) {
        return extensions;
    }

    // default extensions/formats for all
    QSet<QString> formats({"*.zip", "*.7z", "*.ml"});
    QStringList addExts;

#if QT_VERSION >= 0x050e00
    addExts = addExtensions.split(" ", Qt::SkipEmptyParts);
#else
    // for RP on Buster
    addExts = addExtensions.split(" ", QString::SkipEmptyParts);
#endif
    for (auto f : addExts) {
        formats << f;
    }

    QStringList myFormats = peas[platform].toHash()["formats"].toStringList();
    for (auto f : myFormats) {
        if (f.trimmed().startsWith("*.")) {
            formats << f.trimmed().toLower();
        }
    }

    QList<QString> l = formats.values();
    std::sort(l.begin(), l.end());
    QString ret = l.join(" ");
    qDebug() << "getFormats()" << ret;
    return ret;
}

// This contains all known platform aliases as listed on each of the scraping
// source sites
QStringList Platform::getAliases(QString platform) const {
    QStringList aliases;
    // Platform name itself is always appended as the first alias
    aliases.append(platform);
    aliases.append(peas[platform].toHash()["aliases"].toStringList());
    qDebug() << "getAliases():" << aliases;
    return aliases;
}

static inline QStringList splitPlatformIds(const QString &ids) {
    return ids.split("|");
}

bool Platform::parsePlatformsIdCsv(const QString &platformsIdCsvFn) {

    QFile configFile(platformsIdCsvFn);
    const char *fn = platformsIdCsvFn.toUtf8().constData();
    if (!configFile.open(QIODevice::ReadOnly)) {
        if (platformsIdCsvFn == fnPlatformsIdMapLocal) {
            return true; // no platforms_idmap_local.csv, no worries
        }
        printf("\033[1;31mFile not found '%s'. Now quitting...\033[0m\n", fn);
        return false;
    }
    while (!configFile.atEnd()) {
        QString line = QString(configFile.readLine()).trimmed();
        if (line.isEmpty() || line.startsWith("#") ||
            line.startsWith("folder,")) {
            continue;
        }
        QStringList parts = line.split(',');
        if (parts.length() != 4) {
            printf("\033[1;31mFile '%s', line '%s' has not four columns, but "
                   "%d. Please fix. Now quitting...\033[0m\n",
                   fn, parts.join(',').toUtf8().constData(),
                   static_cast<int>(parts.length()));
            configFile.close();
            return false;
        }
        QString pkey = parts[0].trimmed();
        if (pkey.isEmpty()) {
            printf(
                "\033[1;33mFile '%s', line '%s' has empty folder/platform. "
                "Ignoring this line. Please fix to mute this warning.\033[0m\n",
                fn, parts.join(',').toUtf8().constData());
            continue;
        }
        parts.removeFirst();
        QVector<QVector<int>> ids(QVector<QVector<int>>(3));
        int col = 0;
        for (QString id : parts) {
            id = id.trimmed();
            if (!id.isEmpty()) {
                for (const auto &i : splitPlatformIds(id)) {
                    bool ok = false;
                    int tmp = i.toInt(&ok);
                    if (ok && tmp >= -1) {
                        ids[col].append((tmp == 0) ? -1 : tmp);
                    } else {
                        ids[col].append(-1);
                        printf("\033[1;33mFile '%s', line '%s,%s' has "
                               "unparsable or too negative int value '%s' (use "
                               "-1 for unknown platform id). Assumming -1 for "
                               "now, please fix to mute this warning.\033[0m\n",
                               fn, pkey.toUtf8().constData(),
                               parts.join(',').toUtf8().constData(),
                               i.toUtf8().constData());
                    }
                }
            }
            col++;
        }
        platformIdsMap.insert(pkey, ids);
    }
    configFile.close();
    return true;
}

bool Platform::loadPlatformsIdMap() {
    return parsePlatformsIdCsv(fnPlatformsIdMap) &&
           parsePlatformsIdCsv(fnPlatformsIdMapLocal);
}

/*
  Interims function introduced with 3.15.2 to detect pristine platform config
  files.

  Rationale: Pristine files can be savely overwritten with the upstream version.
  Whereas user edits in such files should go into _local files (introduced with
  3.13).
*/
bool Platform::isPlatformCfgfilePristine(const QString &cfgFilePath) {
    QMap<QString, QStringList> sha256sums = {
        // clang-format off
        {"peas.json", QStringList(
                {"67739818ca4d62f277f5c143bff89e0a0083eae96ae26692ec509af5a3db677b",
                 "eb88759262cfa3da46f0f6ff19fba4c89a20c4089ca51294ad4554c6613d9db8",
                 "215c8974fbd2490dedc6e6e59541bc6a36dfb12e8750031aeade99e5e4878c8d",
                 "dfd5591107d585eeecfd3e37beb1b2d80b740caae84ac79d09c65704677740d2",
                 "f046b81f403b132379a4f93e3e5d9482e60b69ce3d18a13539a895ea2d6583d8",
                 "cdcd6abdfdb5b797df183feb03094908bb638f8b2038177769fb73f49caba7e9",
                 "f0dff220a6a07cf1272f00f94d5c55f69353cdce786f8dbfef029dbf30a48a7d",
                 "6c648e3577992caef99c73a6e325a7e9580babf7eafc7ecf35eb349f9da594a1"}
            )
        },
        {"platforms_idmap.csv", QStringList(
                {"78ca2da2de3ee98e57d7ce9bb88504c7b45bdf72a2599a34e583ebcc0855cbef",
                 "30c443a6a6c7583433e62e89febe8d10bae075040e5c1392623a71f348f3f476",
                 "bf12d0f2f7161d45041f8996c44d6c3c2f666cfc33938dbcbd506c1f766062c4",
                 "44a416856327c01c1ec73c41252f9c3318bf703c33fd717935f31b37e635f413",
                 "9af2abea78af7b94b8c86d97417fb4aff347a8b6eef5c0fdab37be31938f5f9a"}
            )
        }
        // clang-format on
    };
    QFileInfo cfgFileInfo = QFileInfo(cfgFilePath);
    if (!cfgFileInfo.exists()) {
        return true;
    }

    QCryptographicHash sha256 = QCryptographicHash(QCryptographicHash::Sha256);
    QFile cfgFile(cfgFilePath);
    bool isPristine = false;
    if (cfgFile.open(QFile::ReadOnly)) {
        sha256.addData(cfgFile.readAll());
        QString currentSha256 = sha256.result().toHex();
        QString cfgBn = cfgFileInfo.fileName();
        isPristine = sha256sums[cfgBn].contains(currentSha256);
        qDebug() << cfgFileInfo.absoluteFilePath() << currentSha256
                 << "is pristine:" << isPristine;
    } else {
        printf("\033[1;31mFile '%s' can not be read. Please fix. "
               "Quitting.\033[0m\n",
               cfgFilePath.toUtf8().constData());
        exit(1);
    }
    return isPristine;
}

QVector<int> Platform::getPlatformIdOnScraper(const QString platform,
                                              const QString scraper) const {
    QVector<int> id;
    if (platformIdsMap.contains(platform)) {
        QVector<QVector<int>> ids = platformIdsMap[platform];
        qDebug() << "platform ids" << ids;
        if (scraper == "screenscraper") {
            id = ids[0];
        } else if (scraper == "mobygames") {
            id = ids[1];
        } else if (scraper == "thegamesdb") {
            id = ids[2];
        }
    }
    qDebug() << "Got platform id(s)" << id << "for platform" << platform
             << "and scraper" << scraper;
    return id;
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
