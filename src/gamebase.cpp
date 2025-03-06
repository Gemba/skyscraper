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

#include "gamebase.h"

#include <QDebug>
#include <QFileInfo>
#include <QStringBuilder>

GamebaseScraper::GamebaseScraper(Settings *config,
                                 QSharedPointer<NetManager> manager)
    : AbstractScraper(config, manager, MatchType::MATCH_MANY) {

    qDebug() << "Provided SQLite3 file" << config->gameBaseFile;
    QFileInfo fi(config->gameBaseFile);
    baseUrl = fi.absolutePath();

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(config->gameBaseFile);
    if (!db.open()) {
        qCritical() << "Connection with database failed. Terminating.";
        qCritical() << db.lastError();
        reqRemaining = 0;
    }

    // supported types:
    // txt
    // AGES, DEVELOPER, PLAYERS, PUBLISHER, RATING, RELEASEDATE, TAGS
    // bin
    // SCREENSHOT, COVER

    // other data is retrieved in one query
    fetchOrder.append(COVER);
}

GamebaseScraper::~GamebaseScraper(){};

static const QString SQL_QUERY_GAMES = R"EOF(
SELECT
    GA_Id AS Id
  , Name
  , Filename
  , Years.Year AS Year
  , Publishers.Publisher AS Publisher
  , CRC
FROM Games
INNER JOIN Years      ON Games.YE_ID = Years.YE_Id
INNER JOIN Publishers ON Games.PU_ID = Publishers.PU_Id
WHERE %1
ORDER BY 2
LIMIT %2
)EOF";

static const QString SQL_ONE_NAME = R"EOF(
SELECT
    GA_Id AS Id
  , Name
  , Filename
  , ScrnshotFilename
  , MAX(PlayersFrom, PlayersTo, 1) AS Players
  , Rating
  , Developers.Developer AS Developer
  , Languages.Language AS Language
  , Genres.Genre AS Genre
  , Adult
  , CRC
FROM Games
INNER JOIN Developers ON Games.DE_ID = Developers.DE_Id
INNER JOIN Languages  ON Games.LA_ID = Languages.LA_Id
INNER JOIN Genres     ON Games.GE_ID = Genres.GE_Id
WHERE GA_Id = (:id)
)EOF";

static const QString SQL_GET_COVER = R"EOF(
SELECT
    MIN(Extras.DisplayOrder) AS DispOrder
  , Extras.Path              AS CoverPath
FROM Extras
JOIN Games ON Extras.GA_Id = Games.GA_ID
WHERE Games.GA_ID = (:id)
AND Extras.Name LIKE 'Cover%'
)EOF";

static const QRegularExpression BRACKETS_RE =
    QRegularExpression("^(\\(|\\[).+(\\)|\\])$");

static const QMap<QString, QString> SQL_CLAUSES = {
    {"Filename", "UPPER(%1) LIKE '%' || UPPER(:v) || '%2.ZIP'"},
    {"Name", "LOWER(%1) %2 LOWER(:v)"},
    {"CRC", "LOWER(%1) = LOWER(:v)"}};

static const QRegularExpression GLOB_PATTERN = QRegularExpression(R"(\*|\?)");

void GamebaseScraper::getSearchResults(QList<GameEntry> &gameEntries,
                                       QString searchName, QString platform) {
    bool results = false;
    QString sql;
    QString clause;
    int limit = 25;
    int rowCtr = 0;
    QString origSearch = searchName;
    for (auto const &c : QStringList({"Filename", "Name", "CRC"})) {
        clause = SQL_CLAUSES[c];
        clause = clause.arg(c);
        if (c == "Name") {
            QString oper = "=";
            if (searchName.contains(GLOB_PATTERN)) {
                searchName.replace("*", "%").replace("?", "_");
                oper = "LIKE";
            }
            clause = clause.arg(oper);
        } else if (c == "Filename") {
            // match PIRATES!_05727_05.zip from gamefile PIRATES!.d64
            clause = clause.arg(searchNameIsFilename ? "%" : "");
        }

        QSqlQuery q;
        q.prepare(SQL_QUERY_GAMES.arg(clause).arg(limit));
        q.bindValue(":v", searchName);
        q.exec();
        qDebug() << "Executed:" << q.lastQuery();
        qDebug() << "v =" << q.boundValue(":v").toString();
        qDebug() << "last error:" << q.lastError();

        results = false;
        rowCtr = 0;
        while (q.next()) {
            GameEntry game;
            int i = q.record().indexOf("Id");
            QString v = q.value(i).toString();
            game.id = v;
            i = q.record().indexOf("Name");
            v = q.value(i).toString();
            game.title = v;
            qDebug() << "Candidate:" << v << "GA_Id:" << game.id;
            i = q.record().indexOf("Year");
            int yy = q.value(i).toInt();
            v = yy > 2100 ? "" : QString::number(yy);
            game.releaseDate = v;
            i = q.record().indexOf("Publisher");
            v = q.value(i).toString();
            game.publisher = v.contains(BRACKETS_RE) ? "" : v;
            game.platform = platform;
            gameEntries.append(game);
            results = true;
            rowCtr++;
        }
        if (results)
            break;
    }
    if (rowCtr == limit) {
        printf("\033[1;33mSearch with '%s' reached limit of %d:\033[0m\nNarrow "
               "down the query parameter, scraping will most likely be not "
               "accurate!\n\n",
               origSearch.toStdString().c_str(), limit);
    }
}

void GamebaseScraper::getGameData(GameEntry &game) {
    QSqlQuery q;
    q.prepare(SQL_ONE_NAME);
    q.bindValue(":id", game.id);
    q.exec();
    qDebug() << "Executed:" << q.lastQuery();
    qDebug() << "id =" << q.boundValue(":id").toString();
    qDebug() << "last error:" << q.lastError();

    q.first();
    int i = q.record().indexOf("Developer");
    QString v = q.value(i).toString();
    game.developer = v.contains(BRACKETS_RE) ? "" : v;
    i = q.record().indexOf("Rating");
    int r = q.value(i).toInt();
    v = r == 0 ? "" : QString::number(r / 5.0f);
    game.rating = v;
    i = q.record().indexOf("Players");
    v = q.value(i).toString();
    game.players = v;
    i = q.record().indexOf("Genre");
    v = q.value(i).toString();
    game.tags = v.contains(BRACKETS_RE) ? "" : v;
    i = q.record().indexOf("Ages");
    game.ages = q.value(i).toInt() ? "18" : "";
    i = q.record().indexOf("ScrnshotFilename");
    v = q.value(i).toString();
    qDebug() << "Screenshot relPath:" << v;
    QByteArray d = loadImageData("Screenshots/", v);
    QImage image;
    if (!d.isEmpty()) {
        game.screenshotData = d;
    }
    q.clear();
    // get cover
    populateGameEntry(game);
}

QList<QString> GamebaseScraper::getSearchNames(const QFileInfo &info,
                                               QString &debug) {
    const QString baseName = info.completeBaseName();
    QString searchName = baseName;

    debug.append("Base name: '" + baseName + "'\n");

    if (QString aliasTitle = lookupAliasMap(baseName, debug);
        aliasTitle != searchName) {
        searchName = aliasTitle;
    } else {
        searchNameIsFilename = true;
    }

    return QList<QString>({searchName});
}

void GamebaseScraper::getCover(GameEntry &game) {
    QSqlQuery q;
    q.prepare(SQL_GET_COVER);
    q.bindValue(":id", game.id);
    q.exec();
    qDebug() << "Executed:" << q.lastQuery();
    qDebug() << "id =" << q.boundValue(":id").toString();
    qDebug() << "last error:" << q.lastError();

    q.first();
    int i = q.record().indexOf("CoverPath");
    QString v = q.value(i).toString();
    qDebug() << "Cover relPath:" << v;
    QByteArray d = loadImageData("Extras/", v);
    if (!d.isEmpty()) {
        game.coverData = d;
    }
    q.clear();
}

QByteArray GamebaseScraper::loadImageData(const QString &subFolder,
                                          QString &fileName) {
    QByteArray data;
    if (fileName.isEmpty())
        return data;

    fileName = fileName.replace("\\", "/");
    QFile binFile(baseUrl % "/" % subFolder % fileName);
    if (binFile.open(QIODevice::ReadOnly)) {
        data = binFile.readAll();
        binFile.close();
        QImage img;
        if (!img.loadFromData(data)) {
            qWarning() << "Unknown image format:"
                       << QFileInfo(binFile).absoluteFilePath();
            data.clear();
        }
    } else {
        qWarning() << "File not readable:"
                   << QFileInfo(binFile).absoluteFilePath();
    }
    return data;
}
