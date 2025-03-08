/***************************************************************************
 *            strtools.cpp
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

#include "strtools.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QRegularExpression>
#include <QStringBuilder>

inline void issueParseWarning(QString msg, QString v) {
    qWarning() << msg << v;
    qWarning()
        << "Please report an issue at https://github.com/Gemba/skyscraper";
    qWarning() << "Use the first line of this warning message as issue title, "
                  "no further details needed. Thank you!";
}

QString StrTools::xmlUnescape(QString str) {
    QMap<QString, QString> xmlMap = {
        {"&amp;", "&"},    {"&lt;", "<"},     {"&gt;", ">"},   {"&quot;", "\""},
        {"&apos;", "'"},   {"&copy;", "(c)"}, {"&#32;", " "},  {"&#33;", "!"},
        {"&#34;", "\""},   {"&#35;", "#"},    {"&#36;", "$"},  {"&#37;", "%"},
        {"&#38;", "&"},    {"&#39;", "'"},    {"&#40;", "("},  {"&#41;", ")"},
        {"&#42;", "*"},    {"&#43;", "+"},    {"&#44;", ","},  {"&#45;", "-"},
        {"&#46;", "."},    {"&#47;", "/"},    {"&#032;", " "}, {"&#033;", "!"},
        {"&#034;", "\""},  {"&#035;", "#"},   {"&#036;", "$"}, {"&#037;", "%"},
        {"&#038;", "&"},   {"&#039;", "'"},   {"&#040;", "("}, {"&#041;", ")"},
        {"&#042;", "*"},   {"&#043;", "+"},   {"&#044;", ","}, {"&#045;", "-"},
        {"&#046;", "."},   {"&#047;", "/"},   {"&#160;", " "}, {"&#179;", "3"},
        {"&#8211;", "-"},  {"&#8217;", "'"},  {"&#xF4;", "o"}, {"&#xE3;", "a"},
        {"&#xE4;", "ae"},  {"&#xE1;", "a"},   {"&#xE9;", "e"}, {"&#xED;", "i"},
        {"&#x16B;", "uu"}, {"&#x22;", "\""},  {"&#x26;", "&"}, {"&#x27;", "'"},
        {"&#xB3;", "3"},   {"&#x14D;", "o"}};

    QMapIterator<QString, QString> i(xmlMap);
    while (i.hasNext()) {
        i.next();
        str = str.replace(i.key(), i.value());
    }

    while (str.contains("&") && str.contains(";") &&
           str.indexOf("&") < str.indexOf(";") &&
           str.indexOf(";") - str.indexOf("&") <= 10) {
        str = str.remove(str.indexOf("&"),
                         str.indexOf(";") + 1 - str.indexOf("&"));
    }
    return str;
}

QString StrTools::xmlEscape(QString str) {
    str = xmlUnescape(str);
    return str.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace("\"", "&quot;")
        .replace("'", "&apos;");
}

QByteArray StrTools::magic(const QByteArray str) {
    QByteArray magicStr("WowMuchEncryptedVeryImpressIGuessThisHasToBeQuiteLongT"
                        "oAlsoSupportSomeVeryLongKeys");

    int strChars[str.length()];
    int magicChars[str.length()];

    for (int a = 0; a < str.length(); ++a) {
        strChars[a] = str.at(a);
    }

    for (int a = 0; a < str.length(); ++a) {
        magicChars[a] = magicStr.at(a);
    }

    QByteArray thingie;
    for (int a = 0; a < str.length(); ++a) {
        thingie.append(QString::number(strChars[a] += magicChars[a]).toUtf8() %
                       ";");
    }
    thingie.chop(1);
    return thingie;
}

QByteArray StrTools::unMagic(const QByteArray str) {
    int length = str.split(';').length();

    QByteArray magicStr("WowMuchEncryptedVeryImpressIGuessThisHasToBeQuiteLongT"
                        "oAlsoSupportSomeVeryLongKeys");

    int strChars[length];
    int magicChars[length];

    for (int a = 0; a < length; ++a) {
        strChars[a] = str.split(';').at(a).toInt();
    }
    for (int a = 0; a < length; ++a) {
        magicChars[a] = magicStr.at(a);
    }
    QByteArray thingie;
    for (int a = 0; a < length; ++a) {
        thingie.append(QString(QChar(strChars[a] -= magicChars[a])).toUtf8());
    }
    return thingie;
}

QString StrTools::conformPlayers(const QString playerString) {
    QString str = playerString.simplified();
    QStringList patterns = {"^(\\d) Player", "^(\\d) Only", "^1 or (\\d)",
                            "^\\d\\s?(?:-|to)\\s?(\\d\\d?)", "^(\\d)\\+"};
    QRegularExpression re;
    re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch m;
    for (auto const &p : patterns) {
        re.setPattern(p);
        m = re.match(str);
        if (m.hasMatch())
            return m.captured(1);
    }

    // number as text
    re.setPattern("^single player");
    if (re.match(str).hasMatch())
        return "1";

    bool ok;
    str.toInt(&ok);
    if (ok)
        return str;

    if (!str.isEmpty())
        issueParseWarning(
            "Player count not replaced to number for scraper input", str);
    return str;
}

QString StrTools::conformAges(QString str) {
    QMap<QString, QString> ageMap = {{"0 (ohne Altersbeschränkung)", "1"},
                                     {"U", "1"},
                                     {"E", "1"},
                                     {"E - Everyone", "1"},
                                     {"Everyone", "1"},
                                     {"GA", "1"},
                                     {"EC", "3"},
                                     {"Early Childhood", "3"},
                                     {"3+", "3"},
                                     {"G", "3"},
                                     {"KA", "6"},
                                     {"Kids to Adults", "6"},
                                     {"G8+", "8"},
                                     {"E10+", "10"},
                                     {"E10+ - Everyone 10+", "10"},
                                     {"Everyone 10+", "10"},
                                     {"11+", "11"},
                                     {"12+", "11"},
                                     {"MA-13", "13"},
                                     {"T", "13"},
                                     {"T - Teen", "13"},
                                     {"Teen", "13"},
                                     {"M", "15"},
                                     {"M15+", "15"},
                                     {"MA 15+", "15"},
                                     {"MA15+", "15"},
                                     {"PG", "15"},
                                     {"15+", "15"},
                                     {"MA-17", "17"},
                                     {"18+", "18"},
                                     {"R18+", "18"},
                                     {"18 (keine Jugendfreigabe)", "18"},
                                     {"A", "18"},
                                     {"AO", "18"},
                                     {"AO - Adults Only", "18"},
                                     {"Adults Only", "18"},
                                     {"M - Mature", "18"},
                                     {"Mature", "18"}};

    QMapIterator<QString, QString> i(ageMap);
    while (i.hasNext()) {
        i.next();
        if (str == i.key())
            return i.value();
    }
    bool ok;
    str.toInt(&ok);
    if (ok)
        return str;

    const QStringList junkList = {
        "not rated" /* #112 */
    };
    if (junkList.contains(str, Qt::CaseInsensitive))
        return "";

    if (!str.isEmpty())
        issueParseWarning("Age rating not replaced to number for scraper input",
                          str);
    return str;
}

QString StrTools::conformReleaseDate(QString str) {
    // cut of time if present
    str = str.replace(QRegularExpression("T\\d{6}$"), "");
    QMap<QString, QString> dateFormats = {
        // clang-format off
        {"^\\d{4}$", "yyyy"},
        {"^\\d{4}-[0-1]{1}\\d{1}$", "yyyy-MM"},
        {"^\\d{4}-[0-1]{1}\\d{1}-[0-3]{1}\\d{1}$", "yyyy-MM-dd"},
        {"^[0-1]{1}\\d{1}/[0-3]{1}\\d{1}/\\d{4}$", "MM/dd/yyyy"},
        {"^\\d{4}-[a-zA-Z]{3}-[0-3]{1}\\d{1}$", "yyyy-MMM-dd"},
        {"^[a-zA-z]{3}, \\d{4}$", "MMM, yyyy"},
        {"^[a-zA-z]{3} [0-3]{1}\\d{1}, \\d{4}$", "MMM dd, yyyy"},
        {"^[12]{1}[012349]{1}[0-9]{2}[0-1]{1}[0-9]{1}[0-3]{1}[0-9]{1}$", "yyyyMMdd"}
        // clang-format on
    };
    QRegularExpression re;
    re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch m;

    QMapIterator<QString, QString> i(dateFormats);
    while (i.hasNext()) {
        i.next();
        re.setPattern(i.key());
        m = re.match(str);
        if (m.hasMatch())
            return QLocale::c().toDate(str, i.value()).toString("yyyyMMdd");
    }

    // 'N/A' and '????' like
    re.setPattern("^(N[.\\/]?A[.]?|[\?]{1,4})$");
    if (re.match(str).hasMatch())
        return "";

    if (str.length() == 4 /* yyyy */ && str.contains("?") /* #108 */)
        return str;

    if (!str.isEmpty())
        issueParseWarning(
            "Release date string is not parsable as date for scraper input",
            str);
    return str;
}

QString StrTools::conformTags(const QString str) {
    QString tags = "";
#if QT_VERSION >= 0x050e00
    QList<QString> tagList = str.split(',', Qt::SkipEmptyParts);
#else
    // RP on Buster
    QList<QString> tagList = str.split(',', QString::SkipEmptyParts);
#endif
    for (auto &tag : tagList) {
        tag = tag.simplified();
        tag = tag.left(1).toUpper() % tag.mid(1, tag.length() - 1);
        tags = tags % tag.simplified() % ", ";
    }
    tags.chop(2);
    return tags;
}

QString StrTools::getVersionHeader() {
    QString headerString =
        "Running Skyscraper v" VERSION " by Lars Muldjord and contributors";
    QString dashesString = "";
    for (int a = 0; a < headerString.length(); ++a) {
        dashesString = dashesString % "-";
    }

    return QString("\033[1;34m" % dashesString % "\033[0m\n\033[1;33m" %
                   headerString % "\033[0m\n\033[1;34m" % dashesString %
                   "\033[0m\n");
}

QString StrTools::getVersionBanner() {
    QStringList g = {"62", "60", "95", "132", "131", "167", "203"};
    int idx = 0;
    QString ver = VERSION;
    QString pad;
    const int fieldSize = 12;
    const int ps = ver.length() - fieldSize;
    bool padded = false;
    while (ps > 0 && pad.length() < ps &&
           pad.length() < 80 - 69 /* width - longest line unpadded */) {
        pad += " ";
        padded = true;
    }
    pad.chop(1);
#ifdef XDG
    QString xdg = "(XDG)";
#else
    QString xdg = "     ";
#endif
    QStringList b = {
        // clang-format off
         g[idx++] % "m _______ __                                                   "%pad%" ___",
         g[idx++] % "m|   _   |  |--.--.--.-----.----.----.---.-.-----.-----.----.  "%pad%"|\"\"\"|",
         g[idx++] % "m|   1___|    <|  |  |__ --|  __|   _|  _  |  _  |  -__|   _|  "%pad%"|\"\"\"|",
         g[idx++] % "m|____   |__|__|___  |_____|____|__| |___._|   __|_____|__|    "%pad%"|\"\"\"|",
        (g[idx++] + "m|:  1   |     |_____|                     |__| %1  %2|\"\"\"|").arg(ver, fieldSize).arg(padded ? "" : " "),
        (g[idx++] + "m|::.. . |                                             %1   "%pad%"|\"\"\"|").arg(xdg),
         g[idx++] % "m`-------' by Lars Muldjord and contributors                  "%pad%"\"\"''''\"",
        // clang-format on
    };
    QString bs = "\b\b\b\b\b\b\b\b\b\b\b";
    return bs % "\033[38;5;" % b.join("\n\033[38;5;") % "\033[0m \n";
}

QString StrTools::stripBrackets(const QString str) {
    return str.left(str.indexOf("(")).left(str.indexOf("[")).simplified();
}

QString StrTools::stripHtmlTags(QString str) {
    while (str.contains("<") && str.contains(">") &&
           str.indexOf("<") < str.indexOf(">")) {
        str = str.remove(str.indexOf("<"),
                         str.indexOf(">") + 1 - str.indexOf("<"));
    }
    return str;
}

QString StrTools::getMd5Sum(const QByteArray &data) {
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(data);
    return md5.result().toHex();
}

QString StrTools::tidyText(QString text, bool ignoreBangs) {
    // remove and replace some artifacts from description text
    text = text.replace("…", "...");
    text = text.replace(".  ", ". ");
    text = text.replace("!  ", "! ");
    text = text.replace("?  ", "? ");
    if (!ignoreBangs) {
        QRegularExpression re("\\!+");
        text = text.replace(re, "!");
    }

    QStringList pars = text.split("\n");
    QStringList po;
    int ctr = 0;
    int tmpCtr = -1;
    bool itemize = false;
    QRegularExpression re("^[\\*●]\\s+");
    QString tmpPiggy;
    for (auto l : pars) {
        ctr++;
        l = l.trimmed();
        if (l.startsWith("* ") || l.startsWith("● ")) {
            if (ctr - 1 == tmpCtr) {
                po.append(tmpPiggy.replace(re, "- "));
                tmpCtr = -1;
                itemize = true;
            } else if (!itemize) {
                tmpPiggy = l;
                tmpCtr = ctr;
                continue;
            }
            if (itemize) {
                po.append(l.replace(re, "- "));
                continue;
            }
        }
        if (tmpCtr > -1) {
            // no list of items, single line beginning with '* ', leave as is
            tmpCtr = -1;
            po.append(tmpPiggy);
        }
        itemize = false;
        po.append(l);
    }
    // if last line
    if (tmpCtr > -1) {
        // no list of items, single line beginning with '* ', leave as is
        tmpCtr = -1;
        po.append(tmpPiggy);
    }
    return po.join("\n");
}
