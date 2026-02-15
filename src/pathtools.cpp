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

#include "pathtools.h"

#include "platform.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringBuilder>
#include <filesystem>

QString PathTools::concatPath(QString path, QString subPath) {
    if (!subPath.isEmpty()) {
        while (subPath.left(1) == "/")
            subPath.remove(0, 1);
    }
    if (!path.isEmpty()) {
        while (path.right(1) == "/")
            path.chop(1);
    }
    if (subPath == "." || subPath.isEmpty())
        return path;
    if (!path.isEmpty() && path.right(1) != "/") {
        return path % "/" % subPath;
    }
    return path % subPath;
}

QString PathTools::makeAbsolutePath(const QString &prePath, QString subPath) {
    if (subPath.isEmpty())
        return subPath;

    Q_ASSERT(QFileInfo(prePath).isAbsolute());

    if (QFileInfo(subPath).isAbsolute() || subPath.startsWith("~/"))
        return subPath;

    if (subPath.startsWith("../"))
        return concatPath(prePath, subPath);

    if (subPath.startsWith("./"))
        subPath.remove(0, 1);

    return concatPath(prePath, subPath);
}

QString PathTools::lexicallyRelativePath(const QString &base,
                                         const QString &other) {
    std::filesystem::path result = std::filesystem::path(other.toStdString())
                                       .lexically_relative(base.toStdString());
    return QString(result.string().c_str());
}

QString PathTools::lexicallyNormalPath(const QString &pathWithDots) {
    // not using QFileInfo::canonicalFilePath() as it would resolve symlinks
    // which is not wanted
    std::filesystem::path result =
        std::filesystem::path(pathWithDots.toStdString()).lexically_normal();
    return QString(result.string().c_str());
}

QString &PathTools::expandHomePath(QString &path) {
    if (path.startsWith("~/")) {
        path.remove(0, 1);
        path = QDir::homePath() % path;
        // "~account/folder" not supported
    }
    return path;
}

const char *PathTools::pathToCStr(QString &in) {
    QString ret = in;
#ifndef Q_OS_WIN
    ret = ret.replace(QDir::homePath(), "~");
#endif
    return ret.toUtf8().constData();
}
