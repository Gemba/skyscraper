/***************************************************************************
 *            nametools.h
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

#ifndef NAMETOOLS_H
#define NAMETOOLS_H

#include "gameentry.h"

#include <QFileInfo>
#include <QObject>

class NameTools : public QObject {
public:
    static QString getScummName(const QFileInfo &info, const QString baseName,
                                const QString scummIni);
    static QString getNameWithSpaces(const QString baseName);
    static QString getUrlQueryName(const QString baseName, const int words = -1,
                                   const QString spaceChar = "+");
    static bool hasArabicNumeral(const QString baseName);
    static bool hasRomanNumeral(const QString baseName);
    static QString convertToArabicNumeral(const QString baseName);
    static QString convertToRomanNumeral(const QString baseName);
    static int getNumeral(const QString baseName);
    static QString getSqrNotes(QString baseName);
    static QString getParNotes(QString baseName);
    static QString getUniqueNotes(const QString &notes, QChar delim);
    static QString getCacheId(const QFileInfo &info);
    static QString getNameFromTemplate(const GameEntry &game,
                                       const QString &nameTemplate);

private:
    static QString notesByRegex(const QString &baseName, const QString &re);
    static const inline QMap<QString, QString> arabicRomanNumerals() {
        return QMap<QString, QString>{
            {"1", "I"},     {"2", "II"},     {"3", "III"},  {"4", "IV"},
            {"5", "V"},     {"6", "VI"},     {"7", "VII"},  {"8", "VIII"},
            {"9", "IX"},    {"10", "X"},     {"11", "XI"},  {"12", "XII"},
            {"13", "XIII"}, {"14", "XIV"},   {"15", "XV"},  {"16", "XVI"},
            {"17", "XVII"}, {"18", "XVIII"}, {"19", "XIX"}, {"20", "XX"}};
    }
};

#endif // NAMETOOLS_H
