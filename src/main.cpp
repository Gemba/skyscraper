/***************************************************************************
 *            main.cpp
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

#include "cli.h"
#include "config.h"
#include "skyscraper.h"
#include "strtools.h"

#include <QCoreApplication>
#include <QDir>
#include <QTimer>
#include <QtGlobal>

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
#include <signal.h>
#elif defined(Q_OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

Skyscraper *x = nullptr;
int sigIntRequests = 0;

void customMessageHandler(QtMsgType type, const QMessageLogContext &,
                          const QString &msg) {
    QString txt;
    // Decide which type of debug message it is, and add string to signify it
    // Then append the debug message itself to the same string.
    switch (type) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    case QtInfoMsg:
        txt += QString(" INFO: %1").arg(msg);
        break;
#endif
    case QtDebugMsg:
        txt += QString("DEBUG: %1").arg(msg);
        break;
    case QtWarningMsg:
        if (msg.contains("NetManager") ||
            msg.contains("iCCP: known incorrect sRGB profile")) {
            /* ugly, needs proper fix: */
            // NetManager "Cannot create children for a parent that is in a
            // different thread."
            /* and */
            // libpng warning: iCCP: known incorrect sRGB profile
            return;
        }
        txt += QString(" WARN: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt += QString(" CRIT: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt += QString("FATAL: %1").arg(msg);
        abort();
    }
    printf("%s\n", txt.toStdString().c_str());
    fflush(stdout);
}

// clang-format off
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
void sigHandler(int signal) {
    if (signal == 2) {
#elif defined(Q_OS_WIN)
BOOL WINAPI ConsoleHandler(DWORD dwType) {
    if (dwType == CTRL_C_EVENT) {
#endif
        sigIntRequests++;
        if (sigIntRequests <= 2) {
            if (x != nullptr) {
                if (x->state == Skyscraper::OpMode::SINGLE) {
                    // Nothing important going on, just exit
                    exit(1);
                } else if (x->state == Skyscraper::OpMode::NO_INTR) {
                    // Ignore signal, something important is going on
                    // that needs to finish!
                } else if (x->state == Skyscraper::OpMode::CACHE_EDIT) {
                    // Cache being edited, clear the queue to quit
                    // nicely
                    x->queue->clearAll();
                    x->state = Skyscraper::OpMode::CACHE_EDIT_DISMISS;
                } else if (x->state == Skyscraper::OpMode::THREADED) {
                    // Threads are running, clear queue for a nice exit
                    printf(
                        "\033[1;33mUser wants to quit, trying to exit "
                        "nicely. This can take a few seconds depending "
                        "on how many threads are running...\033[0m\n");
                    x->queue->clearAll();
                }
            } else {
                exit(1);
            }
        } else {
            printf("\033[1;31mUser REALLY wants to quit NOW, forcing "
                    "unclean exit...\033[0m\n");
            exit(1);
        }
    }
#if defined(Q_OS_WIN)
    return TRUE;
}
#elif defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
}
#endif

int main(int argc, char *argv[]) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = sigHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
#elif defined(Q_OS_WIN)
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);
#endif

    QCoreApplication app(argc, argv);
    app.setApplicationVersion(StrTools::getVersionBanner());

    // Get working directory. If user has specified file(s) on command line we
    // need this.
    QString currentDir = QDir::currentPath();

    // Install the custom debug message handler used by qDebug()
    qInstallMessageHandler(customMessageHandler);

    Config::initSkyFolders();
    Config::setupUserConfig();
    Config::checkLegacyFiles();

    QCommandLineParser parser;
    Cli::createParser(&parser, Config::getSupportedPlatforms());
    parser.process(app);

    if (argc <= 1 || parser.isSet("help") || parser.isSet("h")) {
        parser.showHelp();
    } else {
        x = new Skyscraper(currentDir);
        x->loadConfig(parser);
        QObject::connect(x, &Skyscraper::finished, &app,
                            &QCoreApplication::quit);
        QTimer::singleShot(0, x, SLOT(run()));
    }
    return app.exec();
}
// clang-format on
