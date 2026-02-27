/***************************************************************************
 *            strtools.h
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

#ifndef STRTOOLS_H
#define STRTOOLS_H

#include <QByteArray>
#include <QString>

namespace StrTools {
    QString xmlUnescape(QString str);
    QString xmlEscape(QString str);
    QByteArray magic(const QByteArray str);
    QByteArray unMagic(const QByteArray str);
    QString conformPlayers(const QString str);
    QString conformAges(QString str);
    QString conformReleaseDate(QString str);
    QString conformTags(const QString str);
    QString getVersionHeader();
    QString getVersionBanner();
    QString stripBrackets(const QString str);
    QString stripHtmlTags(QString str);
    QString getMd5Sum(const QByteArray &data);
    QString tidyText(QString text, bool ignoreBangs);
    QString shortenText(QString text, int maxLength);
    QString wrapText(const QString &inText, int width = 80);
};     // namespace StrTools
#endif // STRTOOLS_H
