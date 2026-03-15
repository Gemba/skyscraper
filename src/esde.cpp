/*
 *  This file is part of skyscraper.
 *  Copyright 2024 Gemba @ GitHub
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

#include "esde.h"

#include "emulationstation.h"
#include "xmlreader.h"

#include <QDebug>
#include <QDir>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>

static const QRegularExpression RE_ALT_EMU =
    QRegularExpression("(<alternativeEmulator>)(.*?)(</"
                       "alternativeEmulator>)|(<alternativeEmulator/>)",
                       QRegularExpression::CaseInsensitiveOption |
                           QRegularExpression::DotMatchesEverythingOption);

Esde::Esde() {}

inline const QString baseFolder() {
#if !(defined Q_OS_WIN || defined Q_OS_ANDROID)
    // https://gitlab.com/es-de/emulationstation-de/-/blob/v3.0.2/INSTALL.md#changing-the-application-data-directory
    QString appdata =
        QProcessEnvironment::systemEnvironment().value("ESDE_APPDATA_DIR", "");
    if (!appdata.isEmpty()) {
        return appdata;
    }
#endif
    return QString(QDir::homePath() % "/ES-DE");
}

bool Esde::loadOldGameList(const QString &gameListFileString) {
    XmlReader gameListReader = XmlReader(config->inputFolder);

    QFile esDeGamelist = QFile(gameListFileString);
    altEmu = "";
    // Coding Horror: For some reason ES-DE decided to create a gamelist file
    // with two root XML elements. This clearly violates the XML standard!
    // However, to keep the UX for ES-DE users smooth, this is a workaround. cf.
    // #219 and https://gitlab.com/es-de/emulationstation-de/-/issues/1705
    if (esDeGamelist.open(QIODevice::ReadOnly)) {
        // assume the offending part in first 1K
        QString clutter = esDeGamelist.read(1024);
        esDeGamelist.close();
        QRegularExpressionMatch m = RE_ALT_EMU.match(clutter);
        if (m.hasMatch()) {
            altEmu = m.captured(0).replace("\t", "  ") % "\n";
            qDebug() << "Non-XML compliant ES-DE gamelist detected, applying "
                        "workaround";
            // move orig file
            QString tmpFn;
            if (QTemporaryFile tmpFile; tmpFile.open()) {
                tmpFn = tmpFile.fileName();
            }
            if (!QFile::copy(gameListFileString, tmpFn)) {
                qDebug() << "Copy failed from" << gameListFileString << "to"
                         << tmpFn;
                return false;
            }
            if (!QFile::remove(gameListFileString)) {
                qDebug() << "Remove failed, file" << gameListFileString;
                return false;
            }
            QFile invalidXmlFile = QFile(tmpFn);
            if (invalidXmlFile.open(QIODevice::ReadOnly) &&
                esDeGamelist.open(QIODevice::WriteOnly)) {
                // create a standard compliant XML
                QString part;
                QTextStream esDeOut(&esDeGamelist);
                bool replaced = false;
                while (!invalidXmlFile.atEnd()) {
                    part = invalidXmlFile.read(4096);
                    if (QString tmpPart = part.remove(RE_ALT_EMU);
                        !replaced && tmpPart.length() != part.length()) {
                        replaced = true;
                        esDeOut << tmpPart;
                    } else {
                        esDeOut << part;
                    }
                }
                esDeGamelist.close();
                invalidXmlFile.close();
                qDebug() << "Gamelist now XML compliant, removed temporarily:"
                         << altEmu;
            }
#ifdef QT_NO_DEBUG_OUTPUT
            invalidXmlFile.remove();
#else
            qDebug() << "Original ES-DE Gamelist retained in file" << tmpFn;
#endif
        }
    }
    if (gameListReader.setFile(gameListFileString)) {
        oldEntries = gameListReader.getEntries(extraGamelistTags(true));
        return true;
    }
    return false;
}

void Esde::setConfig(Settings *config) {
    this->config = config;
    if (config->scraper == "cache") {
        config->backcovers = true;
        config->fanart = true;
        config->manuals = true;
    }
}

QStringList Esde::extraGamelistTags(bool isFolder) {
    GameEntry g;
    g.isFolder = isFolder;
    return g.extraTagNames(GameEntry::Format::ESDE, g);
}

QStringList Esde::createEsVariantXml(const GameEntry &entry) {
    (void)entry;
    // ES-DE expects mediafiles to be on a specific location by contract and
    // does not require extra XML elements
    return QStringList();
}

QString Esde::getInputFolder() {
    return QDir::homePath() % "/ROMs/" % config->platform;
}

QString Esde::getGameListFolder() {
    return baseFolder() % "/gamelists/" % config->platform;
}

QString Esde::getMediaFolder() {
    return baseFolder() % "/downloaded_media/" % config->platform;
}

GameEntry::Types Esde::supportedMedia() {
    return GameEntry::Types(GameEntry::BACKCOVER | GameEntry::COVER |
                            GameEntry::FANART | GameEntry::MANUAL |
                            GameEntry::MARQUEE | GameEntry::SCREENSHOT |
                            GameEntry::VIDEO | GameEntry::WHEEL);
}
