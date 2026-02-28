/*
 *  This file is part of skyscraper.
 *  Copyright 2026 Gemba @ GitHub, portions (c) 2018 ReneNyffenegger  @ GitHub
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

#ifndef NOCOLOR_H
#define NOCOLOR_H

#include <QProcessEnvironment>
#include <cstdio>

#ifndef Q_OS_WIN
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#else
#include <io.h>
#define ISATTY _isatty
#define FILENO _fileno
#endif

/* yarked from https://github.com/ReneNyffenegger/about-preprocessor */
#define ELEVENTH_ARGUMENT(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, ...) a11
// Note: With GCC, the preprocessor eliminates the comma in
//   , ## __VA_ARGS__
// IF __VA_ARGS__ is empty.
// (https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html) This is necessary
// to correctly count the arguments
//
/* yarked from https://github.com/ReneNyffenegger/about-preprocessor */
#define COUNT_ARGUMENTS(...)                                                   \
    ELEVENTH_ARGUMENT(dummy, ##__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define ncprintf(FMT, ...)                                                     \
    printVA(COUNT_ARGUMENTS(__VA_ARGS__), FMT, ##__VA_ARGS__)

static const bool noColor([] {
    return !ISATTY(FILENO(stdout)) ||
           QProcessEnvironment::systemEnvironment().value("NO_COLOR", "0") !=
               "0";
}());

void printVA(int nvargs, const char *fmt, ...);

#endif