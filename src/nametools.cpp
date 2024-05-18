/***************************************************************************
 *            nametools.cpp
 *
 *  Tue Feb 20 12:00:00 CEST 2018
 *  Copyright 2018 Lars Muldjord
 *  Copyright 2024 Gemba @ GitHub
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

#include "nametools.h"

#include "strtools.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>

QString NameTools::getScummName(const QFileInfo &info, const QString baseName,
                                const QString scummIni) {

    QStringList paths = {scummIni, QDir::homePath() + "/.scummvmrc",
                         "/opt/retropie/configs/scummvm/scummvm.ini"};
    QString scummIniPath;
    for (QString const &p : paths) {
        if (QFileInfo::exists(p) && QFileInfo(p).isFile()) {
            scummIniPath = p;
            break;
        }
    }
    if (!scummIniPath.isEmpty()) {
        QSettings *settings = new QSettings(scummIniPath, QSettings::IniFormat);
        // test if ROM name is game id
        settings->beginGroup(baseName);
        if (settings->contains("description")) {
            return settings->value("description").toString();
        }
        // test the other way around: ROM name is custom but may contain
        // game id
        if (info.isFile()) {
            QFile romFile = QFile(info.absoluteFilePath());
            if (romFile.open(QIODevice::ReadOnly) && !romFile.atEnd()) {
                QString gameId = romFile.readLine();
                settings->beginGroup(gameId.trimmed());
                if (settings->contains("description")) {
                    return settings->value("description").toString();
                }
            }
        }
    }
    return baseName;
}

QString NameTools::getNameWithSpaces(const QString baseName) {
    // Only perform if name contains no spaces
    if (baseName.indexOf(" ") != -1) {
        return baseName;
    }

    QString withSpaces = "";
    QChar previous;
    for (int a = 0; a < baseName.length(); ++a) {
        QChar current = baseName.at(a);
        if (current == '_' || (a > 4 && baseName.mid(a, 4) == "Demo") ||
            baseName.mid(a, 5) == "-WHDL") {
            break;
        }
        if (a > 0) {
            if (current.isDigit() && (!previous.isDigit() && previous != 'x')) {
                withSpaces.append(" ");
            } else if (current == '&') {
                withSpaces.append(" ");
            } else if (current == 'D') {
                if (baseName.mid(a, 6) == "Deluxe") {
                    withSpaces.append(" ");
                } else if (previous != '3' && previous != '4') {
                    withSpaces.append(" ");
                }
            } else if (current.isUpper()) {
                if (previous.isLetter() && !previous.isUpper()) {
                    withSpaces.append(" ");
                } else if (previous == '&') {
                    withSpaces.append(" ");
                } else if (previous == 'D') {
                    withSpaces.append(" ");
                } else if (previous == 'C') {
                    withSpaces.append(" ");
                } else if (previous == 'O') {
                    withSpaces.append(" ");
                } else if (previous.isDigit()) {
                    withSpaces.append(" ");
                }
            }
        }
        withSpaces.append(current);
        previous = current;
    }
    // printf("withSpaces: '%s'\n", withSpaces.toStdString().c_str());
    return withSpaces;
}

QString NameTools::getUrlQueryName(const QString baseName, const int words,
                                   const QString spaceChar) {
    QString newName = baseName;
    // Remove everything in brackets
    newName = StrTools::stripBrackets(newName);
    // The following is mostly, if not only, used when getting the name from
    // mameMap
    newName = newName.left(newName.indexOf(" / ")).simplified();
    // Always remove everything after a ':' since it's always a subtitle
    // newName = newName.left(newName.indexOf(":")).simplified();
    // Always remove everything after a ' - ' since it's always a subtitle
    // newName = newName.left(newName.indexOf(" - ")).simplified();

    QRegularExpressionMatch match;
    // Remove " rev.X" instances
    match =
        QRegularExpression(
            " rev[.]{0,1}([0-9]{1}[0-9]{0,2}[.]{0,1}[0-9]{1,4}|[IVX]{1,5})$")
            .match(newName);
    if (match.hasMatch() && match.capturedStart(0) != -1) {
        newName = newName.left(match.capturedStart(0)).simplified();
    }
    // Remove versioning instances
    match = QRegularExpression(
                " v[.]{0,1}([0-9]{1}[0-9]{0,2}[.]{0,1}[0-9]{1,4}|[IVX]{1,5})$")
                .match(newName);
    if (match.hasMatch() && match.capturedStart(0) != -1) {
        newName = newName.left(match.capturedStart(0)).simplified();
    }

    // If we have the first game in a series, remove the ' I' for more search
    // results
    if (newName.right(2) == " I") {
        newName.chop(2);
    }

    newName = newName.toLower();

    // Always remove 'the' from beginning or end if equal to or longer than 10
    // chars. If it's shorter the 'the' is of more significance and shouldn't be
    // removed.
    if (newName.length() >= 10) {
        if (newName.simplified().left(4) == "the ") {
            newName = newName.simplified().remove(0, 4);
        }
        QRegularExpressionMatch match;
        match = QRegularExpression(", the( - |:|$)").match(newName);
        if (match.hasMatch()) {
            newName = newName.replace(match.captured(0), " ");
        }
    }
    newName = newName.replace("_", " ");
    newName = newName.replace(" - ", " ");
    newName = newName.replace(",", " ");
    newName = newName.replace("&", "%26");
    newName = newName.replace("+", "");
    // A few game names have faulty "s's". Fix them to "s'"
    newName = newName.replace("s's", "s'");
    newName = newName.replace("'", "%27");
    // Finally change all spaces to requested char. Default is '+' since that's
    // what most search engines seem to understand
    newName = newName.simplified().replace(" ", spaceChar);

    // Implement special cases here
    QMap<QString, QString> special = {
        {"ik", "international+karate"},
        {"arkanoid+revenge+of+doh", "arkanoid%3A+revenge+of+doh"},
        {"lemmings+3", "all+new+world+of+lemmings"}};
    newName = special.value(newName, newName);

    if (words > 0) {
        QList<QString> wordList = newName.split(spaceChar);
        if (wordList.size() > words) {
            newName.clear();
            for (int a = 0; a < words; ++a) {
                newName.append(wordList.at(a) + spaceChar);
            }
            newName = newName.chopped(spaceChar.length());
        }
    }
    return newName;
}

bool NameTools::hasArabicNumeral(const QString baseName) {
    return QRegularExpression(" [0-9]{1,2}([: ]+|$)")
        .match(baseName)
        .hasMatch();
}

bool NameTools::hasRomanNumeral(const QString baseName) {
    return QRegularExpression(" [IVX]{1,5}([: ]+|$)")
        .match(baseName)
        .hasMatch();
}

QString NameTools::convertToRomanNumeral(const QString baseName) {
    QRegularExpressionMatch match;
    QString newName = baseName;

    match = QRegularExpression(" ([0-9]{1,2})([: ]+|$)").match(baseName);
    // Match like " 2" or " 2: yada yada"
    if (match.hasMatch()) {
        QString arabicNumeral = match.captured(1);
        QString r = arabicRomanNumerals().value(arabicNumeral, "");
        if (!r.isEmpty()) {
            newName = newName.replace(arabicNumeral, r);
        }
    }
    return newName;
}

QString NameTools::convertToArabicNumeral(const QString baseName) {
    QRegularExpressionMatch match;
    QString newName = baseName;

    match = QRegularExpression(" ([IVX]{1,5})([: ]+|$)").match(baseName);
    // Match like " X" or " X: yada yada"
    if (match.hasMatch()) {
        QString romanNumeral = match.captured(1);
        QString a = arabicRomanNumerals().key(romanNumeral, "");
        if (!a.isEmpty()) {
            newName = newName.replace(romanNumeral, a);
        }
    }
    return newName;
}

int NameTools::getNumeral(const QString baseName) {
    QRegularExpressionMatch match;
    int numeral = 1;

    // Check for roman numerals
    match = QRegularExpression(" ([IVX]{1,5})([: ]+|$)").match(baseName);
    if (match.hasMatch()) {
        QString roman = match.captured(1);
        numeral =
            arabicRomanNumerals().key(roman, QString::number(numeral)).toInt();
    }

    // Check for european digits
    match = QRegularExpression("([1-9]\\d*)([: ]+|$)").match(baseName);
    if (match.hasMatch()) {
        numeral = match.captured(1).toInt();
    }
    return numeral;
}

QString NameTools::notesByRegex(const QString &baseName, const QString &re) {
    QString notes;
    QRegularExpressionMatchIterator iterMatch =
        QRegularExpression(re).globalMatch(baseName);
    while (iterMatch.hasNext()) {
        QRegularExpressionMatch match = iterMatch.next();
        if (match.hasMatch()) {
            notes.append(match.captured(0));
        }
    }
    return notes;
}

QString NameTools::getSqrNotes(QString baseName) {
    // Get square notes: Pattern to match text in brackets
    QString sqrNotes = notesByRegex(baseName, "\\[([^[\\]]+)\\]");

    // Look for '_tag_' or '[tag]' with the last char optional
    QMap<QString, QString> replacements = {
        {"[_[]{1}(Aga|AGA)[_\\]]{0,1}", "AGA"},
        {"[_[]{1}(Cd32|cd32|CD32)[_\\]]{0,1}", "CD32"},
        {"[_[]{1}(Cdtv|cdtv|CDTV)[_\\]]{0,1}", "CDTV"},
        {"[_[]{1}(Ntsc|ntsc|NTSC)[_\\]]{0,1}", "NTSC"},
        {"(Demo|demo|DEMO)[_\\]]{1}", "Demo"}};
    // Don't add PAL detection as it will also match with "_Palace" and such
    QMapIterator<QString, QString> i(replacements);
    while (i.hasNext()) {
        i.next();
        if (QRegularExpression(i.key()).match(baseName).hasMatch()) {
            sqrNotes.append("[" + i.value() + "]");
        }
    }
    return sqrNotes.simplified();
}

QString NameTools::getParNotes(QString baseName) {
    // Pattern to match text in parenthesis
    QString parNotes = notesByRegex(baseName, "\\(([^()]+)\\)");

    QRegularExpressionMatch match;
    // Add "nDisk" detection
    match = QRegularExpression("([0-9]{1,2})[ ]{0,1}Disk").match(baseName);
    if (match.hasMatch()) {
        parNotes.append("(" + match.captured(1) + " Disk)");
    }
    // Add "CD" detection that DON'T match CD32 and CDTV
    if (QRegularExpression("[_[]{1}CD(?!32|TV)").match(baseName).hasMatch()) {
        parNotes.append("(CD)");
    }
    // Look for language and add it
    match = QRegularExpression(
                "([_[]){1}(De|It|Pl|Fr|Es|Fi|Dk|Gr|Cz){1,10}([_\\]]){0,1}")
                .match(baseName);
    if (match.hasMatch()) {
        parNotes.append("(" + match.captured(2) + ")");
    }
    return parNotes.simplified();
}

QString NameTools::getUniqueNotes(const QString &notes, QChar delim) {
#if QT_VERSION >= 0x050e00
    QList<QString> notesList = notes.split(delim, Qt::SkipEmptyParts);
#else
    QList<QString> notesList = notes.split(delim, QString::SkipEmptyParts);
#endif
    QStringList uniqueList;
    QListIterator<QString> i(notesList);
    while (i.hasNext()) {
        QString txt = i.next().chopped(1);
        if (!uniqueList.contains(txt, Qt::CaseInsensitive)) {
            uniqueList.append(txt);
        }
    }
    if (!uniqueList.isEmpty()) {
        return (delim == '[') ? "[" + uniqueList.join("][") + "]"
                              : "(" + uniqueList.join(")(") + ")";
    }
    return "";
}

QString NameTools::getCacheId(const QFileInfo &info) {
    QCryptographicHash cacheId(QCryptographicHash::Sha1);

    // Use checksum of filename if file is a script or an "unstable" compressed
    // filetype
    bool cacheIdFromData = true;
    if (info.suffix() == "uae" || info.suffix() == "cue" ||
        info.suffix() == "conf" || info.suffix() == "sh" ||
        info.suffix() == "svm" || info.suffix() == "scummvm" ||
        info.suffix() == "mds" || info.suffix() == "zip" ||
        info.suffix() == "7z" || info.suffix() == "gdi" ||
        info.suffix() == "ml" || info.suffix() == "bat" ||
        info.suffix() == "au3" || info.suffix() == "po" ||
        info.suffix() == "dsk" || info.suffix() == "nib") {
        cacheIdFromData = false;
    }
    // If file is larger than 50 MiBs, use filename checksum for cache id for
    // optimization reasons
    if (info.size() > 52428800) {
        cacheIdFromData = false;
    }
    // If file is empty always do checksum on filename
    if (info.size() == 0) {
        cacheIdFromData = false;
    }
    if (cacheIdFromData) {
        QFile romFile(info.absoluteFilePath());
        if (romFile.open(QIODevice::ReadOnly)) {
            while (!romFile.atEnd()) {
                cacheId.addData(romFile.read(1024));
            }
            romFile.close();
        } else {
            printf("Couldn't calculate cache id of rom file '%s', please check "
                   "permissions and try again, now exiting...\n",
                   info.fileName().toStdString().c_str());
            exit(1);
        }
    } else {
        cacheId.addData(info.fileName().toUtf8());
    }

    return cacheId.result().toHex();
}

QString NameTools::getNameFromTemplate(const GameEntry &game,
                                       const QString &nameTemplate) {
    QList<QString> templateGroups = nameTemplate.split(";");
    QString finalName;
    for (auto &templateGroup : templateGroups) {
        bool include = false;
        if (templateGroup.contains("%t") && !game.title.isEmpty()) {
            include = true;
        }
        if (templateGroup.contains("%f") && !game.baseName.isEmpty()) {
            include = true;
        }
        if (templateGroup.contains("%b") && !game.parNotes.isEmpty()) {
            include = true;
        }
        if (templateGroup.contains("%B") && !game.sqrNotes.isEmpty()) {
            include = true;
        }
        if (templateGroup.contains("%a") && !game.ages.isEmpty()) {
            include = true;
        }
        if (templateGroup.contains("%d") && !game.developer.isEmpty()) {
            include = true;
        }
        if (templateGroup.contains("%p") && !game.publisher.isEmpty()) {
            include = true;
        }
        if (templateGroup.contains("%P") && !game.players.isEmpty()) {
            include = true;
        }
        if (templateGroup.contains("%D") && !game.releaseDate.isEmpty()) {
            include = true;
        }
        if (include) {
            templateGroup.replace("%t", game.title);
            templateGroup.replace("%f", StrTools::stripBrackets(game.baseName));
            templateGroup.replace("%b", game.parNotes);
            templateGroup.replace("%B", game.sqrNotes);
            templateGroup.replace("%a", game.ages);
            templateGroup.replace("%d", game.developer);
            templateGroup.replace("%p", game.publisher);
            templateGroup.replace("%P", game.players);
            templateGroup.replace("%D", game.releaseDate);
            finalName.append(templateGroup);
        }
    }

    return finalName;
}
