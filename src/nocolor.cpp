/*
 *  This file is part of skyscraper.
 *  Copyright 2026 Gemba @ GitHub
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

#include "nocolor.h"

#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <stdarg.h>
#include <stdio.h>

static const QRegularExpression RE_ANSI_CODE =
    QRegularExpression("\\033[[\\d;]+m");

// capture "%d" with some variants and "%s"
static const QRegularExpression RE_FORMAT =
    QRegularExpression("(%(0?)(\\d?)d|%([\\d]*)s)");

void printVA(int nvargs, const char *fmt, ...) {
    va_list args;
    if (noColor) {
        // remove ANSI from format string
        QString fmtPlain = QString(fmt);
        fmtPlain.remove(RE_ANSI_CODE);
        va_start(args, fmt);
        QRegularExpressionMatch m;
        // replace with actual arg values and remove ANSI controls
        for (int i = 0; i < nvargs; i++) {
            m = RE_FORMAT.match(fmtPlain);
            if (m.hasMatch()) {
                if (m.captured(1).endsWith("d")) {
                    // int %d
                    int val = va_arg(args, int);
                    if (QString width = m.captured(3); !width.isEmpty()) {
                        // flushright
                        if (QString pad = m.captured(2); !pad.isEmpty()) {
                            fmtPlain.replace(
                                m.capturedStart(1), m.capturedLength(1),
                                QString("%1").arg(val, width.toInt(), 10,
                                                  QChar(pad[0])));

                        } else {
                            // no pad char
                            fmtPlain.replace(
                                m.capturedStart(1), m.capturedLength(1),
                                QString("%1").arg(val, width.toInt()));
                        }
                    } else {
                        // flushleft
                        fmtPlain.replace(m.capturedStart(1),
                                         m.capturedLength(1),
                                         QString::number(val));
                    }
                } else {
                    // string %s
                    QString va =
                        QString(va_arg(args, char *)).remove(RE_ANSI_CODE);
                    if (QString width = m.captured(4); !width.isEmpty()) {
                        // flushright
                        fmtPlain.replace(m.capturedStart(1),
                                         m.capturedLength(1),
                                         QString("%1").arg(va, width.toInt()));
                    } else {
                        // flushleft
                        fmtPlain.replace(m.capturedStart(1),
                                         m.capturedLength(1), va);
                    }
                }
            }
        }
        printf("%s", fmtPlain.toStdString().c_str());
    } else {
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
}
