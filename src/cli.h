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

#ifndef CLI_H
#define CLI_H

#include <QCommandLineParser>
#include <QString>

namespace Cli {
    void createParser(QCommandLineParser *parser, const QString platforms);
    void subCommandUsage(const QString subCmd);
    QMap<QString, QString> getSubCommandOpts(const QString subCmd);
    void cacheReportMissingUsage();
    void showHint();
} // namespace Cli

#endif // CLI_H