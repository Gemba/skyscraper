/***************************************************************************
 *            abstractfrontend.cpp
 *
 *  Wed Jun 18 12:00:00 CEST 2017
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

#include "abstractfrontend.h"

#include "config.h"
#include "gameentry.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStringBuilder>

AbstractFrontend::AbstractFrontend() {}

AbstractFrontend::~AbstractFrontend() {}

void AbstractFrontend::setConfig(Settings *config) { this->config = config; }

void AbstractFrontend::sortEntries(QList<GameEntry> &gameEntries) {
    printf("Sorting entries...");
    int dots = 0;
    std::sort(gameEntries.begin(), gameEntries.end(),
              [&dots](const GameEntry a, const GameEntry b) -> bool {
                  if (dots % 1000 == 0) {
                      printf(".");
                      fflush(stdout);
                  }
                  dots++;
                  QString firstTitle = a.title.toLower();
                  QString secondTitle = b.title.toLower();
                  if (firstTitle.left(4) == "the ") {
                      firstTitle.remove(0, 4);
                  }
                  if (secondTitle.left(4) == "the ") {
                      secondTitle.remove(0, 4);
                  }

                  return firstTitle < secondTitle;
              });
    printf(" \033[1;32mDone!\033[0m\n");
}

QString AbstractFrontend::getTargetFilePath(const GameEntry::Types t,
                                            const QString &baseName,
                                            const QString &subPath,
                                            const QString &cacheFn,
                                            QString ext) {
    QString fnExt = ext;
    if (ext.isEmpty()) {
        QMimeType mime = mimeDb.mimeTypeForFile(cacheFn);
        fnExt = mime.preferredSuffix().replace("jpeg", "jpg");
    }
    QString fp;
    switch (t) {
    case GameEntry::BACKCOVER:
        fp = getBackcoversFolder();
        break;
    case GameEntry::COVER:
        fp = getCoversFolder();
        break;
    case GameEntry::FANART:
        fp = getFanartsFolder();
        break;
    case GameEntry::MANUAL:
        fp = getManualsFolder();
        break;
    case GameEntry::MARQUEE:
        fp = getMarqueesFolder();
        break;
    case GameEntry::SCREENSHOT:
        fp = getScreenshotsFolder();
        break;
    case GameEntry::TEXTURE:
        fp = getTexturesFolder();
        break;
    case GameEntry::VIDEO:
        fp = getVideosFolder();
        break;
    case GameEntry::WHEEL:
        fp = getWheelsFolder();
        break;
    default:
        break;
    }
    QString fn = getTargetFileName(t, baseName);
    if (!fp.isEmpty() && !fn.isEmpty()) {
        fp = fp % QString("/%1/%2.%3").arg(subPath).arg(fn).arg(fnExt);
        QFileInfo fi = QFileInfo(fp);
        QDir d = QDir(fi.absolutePath());
        if (!d.exists()) {
            if (!d.mkpath(".")) {
                qWarning() << "Path could not be created" << fi.absolutePath()
                           << " Check file permissions, gamelist binary data "
                              "maybe incomplete.";
            }
        }
        fp = Config::lexicallyNormalPath(fp);
    } else {
        fp = "";
    }
    return fp;
}

bool AbstractFrontend::copyMedia(GameEntry::Types &savedMedia,
                                 const QString &baseName,
                                 const QString &subPath, GameEntry &game) {
    bool copyError = false;
    bool success = false;
    GameEntry::Types toCopy =
        supportedMedia() & (savedMedia ^ GameEntry::MEDIA);

    if (!config->backcovers)
        toCopy ^= GameEntry::BACKCOVER;
    if (!config->fanart)
        toCopy ^= GameEntry::FANART;
    if (!config->manuals)
        toCopy ^= GameEntry::MANUAL;
    if (!config->videos)
        toCopy ^= GameEntry::VIDEO;

    qDebug() << "toCopy" << toCopy;
    QList<MediaProps> medias;

    if (GameEntry::BACKCOVER & toCopy) {
        MediaProps m =
            MediaProps(GameEntry::BACKCOVER, game.backcoverData,
                       game.backcoverFile, config->skipExistingBackcovers);
        medias.append(m);
    }

    if (GameEntry::COVER & toCopy) {
        MediaProps m = MediaProps(GameEntry::COVER, game.coverData,
                                  game.coverFile, config->skipExistingCovers);
        medias.append(m);
    }
    if (GameEntry::FANART & toCopy) {
        MediaProps m = MediaProps(GameEntry::FANART, game.fanartData,
                                  game.fanartFile, config->skipExistingFanart);
        medias.append(m);
    }
    if (GameEntry::MANUAL & toCopy) {
        MediaProps m = MediaProps(GameEntry::MANUAL, game.manualData,
                                  game.manualFile, config->skipExistingManuals);
        medias.append(m);
    }
    if ((GameEntry::MARQUEE | GameEntry::WHEEL) & toCopy) {
        if (config->frontend == "attractmode" ||
            config->frontend == "pegasus") {
            if (GameEntry::MARQUEE & toCopy && !game.marqueeFile.isEmpty() &&
                !game.marqueeData.isEmpty()) {
                MediaProps m =
                    MediaProps(GameEntry::MARQUEE, game.marqueeData,
                               game.marqueeFile, config->skipExistingMarquees);
                medias.append(m);
            }
            if (GameEntry::WHEEL & toCopy && !game.wheelFile.isEmpty() &&
                !game.wheelData.isEmpty()) {
                MediaProps m =
                    MediaProps(GameEntry::WHEEL, game.wheelData, game.wheelFile,
                               config->skipExistingWheels);
                medias.append(m);
            }
        } else if (!(toCopy & GameEntry::WHEEL) ||
                   toCopy & GameEntry::MARQUEE) {
            // copy only if marquee is set, but not if wheel is set alone:
            // in the latter marquee contains already wheel data via
            // artwork, that is a historical ES-theme misconception.
            // PENDING: In a edge case when no marquee is defined in the artwork
            // this assumption fails
            bool putInGamelist = gamelistHasMediaPaths();
            QString tgt;
            if (!game.wheelFile.isEmpty() && !game.wheelData.isEmpty()) {
                tgt = getTargetFilePath(GameEntry::MARQUEE, baseName, subPath,
                                        game.wheelFile);
                if (!tgt.isEmpty()) {
                    success =
                        doCopy(GameEntry::MARQUEE, game.wheelFile, tgt,
                               game.wheelData, config->skipExistingMarquees);
                    copyError |= !success;
                    putInGamelist &= success;
                }
            }
            if (!putInGamelist || tgt.isEmpty()) {
                game.marqueeFile.clear();
                game.marqueeData.clear();
            } else {
                game.marqueeFile = tgt;
                // avoid output of wheel in gamelist
                game.wheelFile.clear();
                game.wheelData.clear();
            }
        }
    }
    if (GameEntry::SCREENSHOT & toCopy) {
        MediaProps m =
            MediaProps(GameEntry::SCREENSHOT, game.screenshotData,
                       game.screenshotFile, config->skipExistingScreenshots);
        medias.append(m);
    }
    if (GameEntry::TEXTURE & toCopy) {
        MediaProps m =
            MediaProps(GameEntry::TEXTURE, game.textureData, game.textureFile,
                       config->skipExistingTextures);
        medias.append(m);
    }
    if (GameEntry::VIDEO & toCopy) {
        MediaProps m = MediaProps(GameEntry::VIDEO, game.videoData,
                                  game.videoFile, config->skipExistingVideos);
        m.ext = game.videoFormat;
        medias.append(m);
    }

    for (auto &mm : medias) {
        bool putInGamelist = gamelistHasMediaPaths();
        QString tgt;
        // qDebug() << mm.type;
        // qDebug() << *mm.file;
        // qDebug() << mm.data->size();
        if (!mm.file->isEmpty() && !mm.data->isEmpty()) {
            tgt = getTargetFilePath(mm.type, baseName, subPath, *mm.file);
            qDebug() << "tgt" << tgt;
            if (!tgt.isEmpty()) {
                success = doCopy(mm.type, *mm.file, tgt, *mm.data, mm.skip);
                copyError |= !success;
                putInGamelist &= success;
            }
        }
        mm.file->clear();
        if (!putInGamelist || tgt.isEmpty()) {
            mm.data->clear();
        } else {
            mm.file->append(tgt);
        }
    }

    GameEntry::Types drop = (toCopy | savedMedia) ^ GameEntry::MEDIA;
    qDebug() << "drop" << drop;
    if (drop & GameEntry::BACKCOVER)
        game.backcoverFile.clear();
    if (drop & GameEntry::COVER)
        game.coverFile.clear();
    if (drop & GameEntry::FANART)
        game.fanartFile.clear();
    if (drop & GameEntry::MANUAL)
        game.manualFile.clear();
    if (drop & GameEntry::MARQUEE)
        game.marqueeFile.clear();
    if (drop & GameEntry::SCREENSHOT)
        game.screenshotFile.clear();
    if (drop & GameEntry::TEXTURE)
        game.textureFile.clear();
    if (drop & GameEntry::VIDEO)
        game.videoFile.clear();
    if (drop & GameEntry::WHEEL)
        game.wheelFile.clear();

    return copyError;
}

bool AbstractFrontend::doCopy(GameEntry::Types t, const QString &cacheFn,
                              QString &tgt, const QByteArray &data,
                              bool skipExisting) {
    bool success = skipExisting;
    QString altTgt = tgt;
    if (t & GameEntry::IMAGE) {
        // depending on source it may have PNG or JPG
        // format: prepare remove potential leftovers from other source too
        if (tgt.endsWith(".jpg")) {
            altTgt = altTgt.replace(altTgt.length() - 3, 3, "png");
        } else if (tgt.endsWith(".png")) {
            altTgt = altTgt.replace(altTgt.length() - 3, 3, "jpg");
        }
    }
    if (!(skipExisting && (QFile::exists(tgt) || QFile::exists(altTgt)))) {
        QFile::remove(tgt);
        if (GameEntry::Elem::VIDEO & t) {
            if (config->symlink) {
                // symlink
                if (success = QFile::link(cacheFn, tgt); !success) {
                    qWarning() << "Symlink failed, media entry will be not in "
                                  "game list:"
                               << tgt << "->" << cacheFn;
                }
            } else {
                if (success = QFile::copy(cacheFn, tgt); !success) {
                    qWarning() << "Copy video failed, entry will be not in "
                                  "game list:"
                               << cacheFn << "to" << tgt;
                }
            }
        } else {
            if (t & GameEntry::IMAGE) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                bool diff = tgt.last(3) != altTgt.last(3);
#else
                bool diff = tgt.right(3) != altTgt.right(3);
#endif
                if (diff)
                    QFile::remove(altTgt);
            }
            QFile fh(tgt);
            if (success = fh.open(QIODevice::WriteOnly); success) {
                fh.write(data);
                fh.close();
            } else {
                qWarning()
                    << "Copy failed, media entry will be not in game list:"
                    << cacheFn << "to" << tgt;
            }
        }
    }
    if (success) {
        qDebug() << "Copied" << t;
    }
    return success;
}