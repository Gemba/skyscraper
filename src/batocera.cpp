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

#include <QDir>
#include <QStringBuilder>
#include <QStringList>

Batocera::Batocera() {
    // always on
    config->relativePaths = true;
}

static const QString baseFolder() { return QString("/userdata/roms/"); }

inline const QStringList pathGamelistElems() {
    return QStringList({GameEntry::elements()[GameEntry::Elem::COVER],
                        GameEntry::elements()[GameEntry::Elem::SCREENSHOT],
                        GameEntry::elements()[GameEntry::Elem::MARQUEE],
                        GameEntry::elements()[GameEntry::Elem::FANART],
                        GameEntry::elements()[GameEntry::Elem::MANUAL],
                        GameEntry::elements()[GameEntry::Elem::VIDEO],
                        "boxback", "boxart", "map", "bezel", "cartridge",
                        "titleshot", "magazine", "mix", "music"});
};

QStringList Batocera::extraGamelistTags(bool isFolder) {
    (void)isFolder;
    // none, will be detected dynamically
    return QStringList();
}

QStringList Batocera::createEsVariantXml(const GameEntry &entry) {
    QStringList l;
    // at this point entry has already added the oldEntry elements and attribs
    QStringList elemNames =
        entry.extraTagNames(GameEntry::Format::BATOCERA, entry);

    const QMap<QString, QString> scrapedValues = {
        {GameEntry::elements()[GameEntry::Elem::COVER], entry.coverFile},
        {GameEntry::elements()[GameEntry::Elem::SCREENSHOT],
         entry.screenshotFile},
        {GameEntry::elements()[GameEntry::Elem::MARQUEE], entry.marqueeFile},
        {GameEntry::elements()[GameEntry::Elem::FANART], "" /* FIXME */},
        {GameEntry::elements()[GameEntry::Elem::MANUAL], entry.manualFile},
        {GameEntry::elements()[GameEntry::Elem::VIDEO], entry.videoFile}};

    for (auto const &t : elemNames) {
        if (pathGamelistElems().contains(t)) {
            if (scrapedValues.keys().contains(t)) {
                l.append(elem(t, scrapedValues[t], addEmptyElem(), true));
            } else {
                l.append(elem(t, entry.getEsExtra(t), addEmptyElem(), true));
            }
        } else {
            l.append(elemWithAttribs(t, entry.getEsExtraAttribs(t)));
        }
    }
    return l;
}

QString Batocera::openingElement(GameEntry &entry) {
    // PENDING: Assuming Bato allows <folder/> elements
    QString entryType = QString(entry.isFolder ? "folder" : "game");
    if (config->scraper == "screenscraper") {
        return QString("  <" % entryType % " id=\"" % entry.id % "\">");
    } else {
        return QString("  <" % entryType % ">");
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
        return QString("    <%1/>").arg(elemWithAttrs);
    }
    return QString("    <%1>%2</%3>").arg(elemWithAttrs, elemVal, t);
}

void Batocera::preserveVariants(const GameEntry &oldEntry, GameEntry &entry) {
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

QString Batocera::getInputFolder() { return baseFolder() % config->platform; }

QString Batocera::getGameListFolder() {
    return baseFolder() % config->platform;
}

QString Batocera::getMediaFolder() { return baseFolder() % config->platform; }
