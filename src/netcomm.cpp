/***************************************************************************
 *            netcomm.cpp
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

#include "netcomm.h"

#include <QDebug>
#include <QNetworkRequest>
#include <QUrl>

constexpr int DL_MAXSIZE = 100 * 1000 * 1000;

NetComm::NetComm(QSharedPointer<NetManager> manager, int timeout)
    : manager(manager), timeout(timeout) {
    requestTimer.setSingleShot(true);
    connect(&requestTimer, &QTimer::timeout, this, &NetComm::requestTimeout);
}

void NetComm::request(QString query, QString postData,
                      QList<QPair<QString, QString>> headers) {
    QUrl url(query);
    QNetworkRequest request(url);

    QString ua = "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:74.0) "
                 "Gecko/20100101 Firefox/74.0";
    if (!headers.isEmpty()) {
        for (const auto &header : headers) {
            if (header.first == "User-Agent") {
                ua = header.second.toUtf8();
                continue;
            }
            request.setRawHeader(header.first.toUtf8(), header.second.toUtf8());
        }
    }
    request.setHeader(QNetworkRequest::UserAgentHeader, ua);

    if (postData.isNull()) {
        // GET iff postData is null, as "" is in use for POST w/o postData
        // No body -> no Content-Type
        reply = manager->getRequest(request);
    } else if (postData == "HEAD") {
        reply = manager->headRequest(request);
    } else {
        // POST
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          "application/x-www-form-urlencoded");
        reply = manager->postRequest(request, postData.toUtf8());
    }
    connect(reply, &QNetworkReply::finished, this, &NetComm::replyReady);
    connect(reply, &QNetworkReply::downloadProgress, this,
            &NetComm::dataDownloaded);
    requestTimer.start(timeout * 1000);
}

void NetComm::replyReady() {
    requestTimer.stop();
    data = reply->readAll();
    error = reply->error();
    contentType = reply->rawHeader("Content-Type");
    redirUrl = reply->rawHeader("Location");
    headerPairs = reply->rawHeaderPairs();
    httpStatus =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater();
    emit dataReady();
}

QByteArray NetComm::getData() { return data; }

QString NetComm::getHeaderValue(const QString headerKey) {
    for (const auto &h : headerPairs) {
        if (h.first == headerKey.toUtf8()) {
            qDebug() << h.first << ":" << h.second;
            return QString(h.second);
        }
    }
    return "";
}

QNetworkReply::NetworkError NetComm::getError(const int &verbosity) {
    if (!ok() && verbosity >= 1) {
        printf("\033[1;31mNetwork error: ");
        switch (error) {
        case QNetworkReply::RemoteHostClosedError:
            // 'screenscraper' will often give this error when it's overloaded.
            // But since we retry a couple of times, it's rarely a problem.
            printf("'QNetworkReply::RemoteHostClosedError', scraping module "
                   "service might be overloaded.");
            break;
        case QNetworkReply::TimeoutError:
            printf("'QNetworkReply::TimeoutError'");
            break;
        case QNetworkReply::NetworkSessionFailedError:
            printf("'QNetworkReply::NetworkSessionFailedError'");
            break;
        case QNetworkReply::ContentNotFoundError:
            // Don't show an error on these. For some modules I am guessing for
            // urls and sometimes they simply don't exist. It's not an error in
            // those cases.
            // printf("Network error:
            // 'QNetworkReply::ContentNotFoundError'");
            break;
        case QNetworkReply::ContentReSendError:
            printf("'QNetworkReply::ContentReSendError'");
            break;
        case QNetworkReply::ContentGoneError:
            printf("'QNetworkReply::ContentGoneError'");
            break;
        case QNetworkReply::InternalServerError:
            printf("'QNetworkReply::InternalServerError'");
            break;
        case QNetworkReply::UnknownNetworkError:
            printf("'QNetworkReply::UnknownNetworkError'");
            break;
        case QNetworkReply::UnknownContentError:
            printf("'QNetworkReply::UnknownContentError'");
            break;
        case QNetworkReply::UnknownServerError:
            printf("'QNetworkReply::UnknownServerError'");
            break;
        case QNetworkReply::AuthenticationRequiredError:
            printf("'QNetworkReply::AuthenticationRequiredError'");
            break;
        default:
            printf("'%d' (QNetworkReply::NetworkError)", error);
            break;
        }
        printf(" (HTTP status: %d)\033[0m\n", getHttpStatus());
    }
    return error;
}

QByteArray NetComm::getContentType() { return contentType; }

QByteArray NetComm::getRedirUrl() { return redirUrl; }

void NetComm::dataDownloaded(qint64 bytesReceived, qint64 /* bytesTotal */) {
    if (bytesReceived > DL_MAXSIZE) {
        printf("Retrieved data size (%d MB) exceeded maximum of %d MB, "
               "cancelling network request...\n",
               static_cast<int>(bytesReceived / (1000 * 1000)),
               DL_MAXSIZE / (1000 * 1000));
        reply->abort();
    }
}

void NetComm::requestTimeout() {
    printf("\033[1;33mRequest timed out after %d secs, server might be busy / "
           "overloaded...\033[0m\n",
           timeout);
    reply->abort();
}

void NetComm::setTimeout(int secs) { timeout = secs; }
