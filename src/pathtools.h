/*
 *  This file is part of skyscraper.
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

#ifndef PATHTOOLS_H
#define PATHTOOLS_H

#include <QString>

namespace PathTools {
    QString concatPath(QString absPath, QString subPath);
    QString makeAbsolutePath(const QString &prePath, QString subPath);
    QString lexicallyRelativePath(const QString &base, const QString &other);
    QString lexicallyNormalPath(const QString &pathWithDots);
    QString &expandHomePath(QString &path);
    const char *pathToCStr(QString &in);
} // namespace PathTools
#endif // PATHTOOLS_H