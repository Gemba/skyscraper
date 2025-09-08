/*
 *  This file is part of skyscraper.
 *  Copyright 2025 Gemba @ GitHub
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

#include "batocera.h"

#include "emulationstation.h"
#include "gameentry.h"

#include <QDebug>
#include <QDir>
#include <QStringBuilder>
#include <QStringList>

Batocera::Batocera() {}

static const QString baseFolder() { return QString("/userdata/roms/"); }

inline const QStringList binaryGamelistElems() {
    // using enum GameEntry::Elem; TODO --> std-c++-20 onwards
    return QStringList({GameEntry::getTag(GameEntry::Elem::COVER, true),
                        GameEntry::getTag(GameEntry::Elem::SCREENSHOT),
                        GameEntry::getTag(GameEntry::Elem::MARQUEE),
                        GameEntry::getTag(GameEntry::Elem::FANART),
                        GameEntry::getTag(GameEntry::Elem::MANUAL),
                        GameEntry::getTag(GameEntry::Elem::VIDEO), "bezel",
                        "boxart", "boxback", "cartridge", "magazine", "map",
                        "mix", "music", "thumbnail", "titleshot"});
};

void Batocera::setConfig(Settings *config) {
    this->config = config;
    if (config->scraper == "cache") {
        // only enable for gamelist creation
        config->relativePaths = true;
        config->manuals = true;
        config->fanart = true;
    }
}

QString Batocera::getInputFolder() { return baseFolder() % config->platform; }

QString Batocera::getGameListFolder() {
    return baseFolder() % config->platform;
}

QString Batocera::getMediaFolder() { return baseFolder() % config->platform; }

QString Batocera::getCoversFolder() { return config->mediaFolder % "/images"; }
QString Batocera::getScreenshotsFolder() { return getCoversFolder(); }
QString Batocera::getWheelsFolder() { return getCoversFolder(); }
QString Batocera::getMarqueesFolder() { return getCoversFolder(); }
QString Batocera::getTexturesFolder() { return getCoversFolder(); }
QString Batocera::getFanartsFolder() { return getCoversFolder(); }

QStringList Batocera::extraGamelistTags(bool isFolder) {
    (void)isFolder;
    // none, will be detected dynamically per each <game/>
    return QStringList();
}

QStringList Batocera::createEsVariantXml(const GameEntry &entry) {
    QStringList l;
    // at this point entry has already added the oldEntry elements and attribs
    QStringList elemNames =
        entry.extraTagNames(GameEntry::Format::BATOCERA, entry);

    const QMap<QString, QString> scrapedBinsMap = {
        {GameEntry::getTag(GameEntry::Elem::COVER, true), entry.coverFile},
        {GameEntry::getTag(GameEntry::Elem::SCREENSHOT), entry.screenshotFile},
        {GameEntry::getTag(GameEntry::Elem::MARQUEE), entry.marqueeFile},
        {GameEntry::getTag(GameEntry::Elem::FANART), entry.fanartFile},
        {GameEntry::getTag(GameEntry::Elem::MANUAL), entry.manualFile},
        {GameEntry::getTag(GameEntry::Elem::VIDEO), entry.videoFile}};

    for (auto const &el : scrapedBinsMap.keys()) {
        l.append(elem(el, scrapedBinsMap[el], addEmptyElem(), true));
    }

    for (auto const &el : elemNames) {
        if (binaryGamelistElems().contains(el)) {
            if (!scrapedBinsMap.keys().contains(el)) {
                // write back binaries which are currently not scraped by
                // Skyscraper
                l.append(elem(el, entry.getEsExtra(el), addEmptyElem(), true));
            } else {
                qWarning() << "Twin element" << el
                           << "detected in extra-elements for" << entry.path;
            }
        } else {
            // write back all other batocera specific elements with attributes
            l.append(elemWithAttribs(el, entry.getEsExtraAttribs(el)));
        }
    }
    return l;
}

QString Batocera::openingElement(GameEntry &entry) {
    // PENDING: Assuming Bato uses/allows <folder/> elements
    QString entryType = QString(entry.isFolder ? "folder" : "game");
    if (config->frontend == "batocera" && !entry.id.isEmpty()) {
        return QString(INDENT % "<%1 id=\"%2\">").arg(entryType).arg(entry.id);
    } else {
        return QString(INDENT % "<%1>").arg(entryType);
    }
}

QString
Batocera::elemWithAttribs(const QString &t,
                          const QPair<QString, QDomNamedNodeMap> &elemAttribs) {
    const QString elemVal = elemAttribs.first;
    QDomNamedNodeMap attribs = elemAttribs.second;
    if (attribs.isEmpty()) {
        return elem(t, elemVal, addEmptyElement());
    }
    QStringList attrs;
    for (int i = 0; i < attribs.length(); i++) {
        QDomAttr attr = attribs.item(i).toAttr();
        attrs.append(QString("%1=\"%2\"").arg(attr.name(), attr.value()));
    }
    QString elemWithAttrs = t % " " % attrs.join(" ");
    if (elemVal.isEmpty()) {
        return QString(INDENT % INDENT % "<%1/>").arg(elemWithAttrs);
    }
    return QString(INDENT % INDENT % "<%1>%2</%3>")
        .arg(elemWithAttrs, elemVal, t);
}

void Batocera::preserveVariants(const GameEntry &oldEntry, GameEntry &entry) {
    entry.id = oldEntry.id;
    QStringList elemNames =
        oldEntry.extraTagNames(GameEntry::Format::BATOCERA, oldEntry);
    for (const auto &t : elemNames) {
        // exclude the baseline elements are already excluded
        // add if not set in scraped/updated entry
        if (entry.getEsExtra(t).isEmpty()) {
            QPair<QString, QDomNamedNodeMap> p = oldEntry.getEsExtraAttribs(t);
            entry.setEsExtra(t, p.first, p.second);
        }
    }
}
