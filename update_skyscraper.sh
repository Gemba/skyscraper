#! /usr/bin/env bash

# This file is part of skyscraper.
# (c) Lars Muldjord and contributors
#
# skyscraper is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# skyscraper is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with skyscraper; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.

{
    LATEST=$(wget -q -O - "https://api.github.com/repos/Gemba/skyscraper/releases/latest" | python3 -c 'import sys; import json; print(json.loads(sys.stdin.read())["tag_name"])')
    [[ -z "$LATEST" ]] && printf '%s\n' "--- Remote server unreachable. Check internet connectivity. Exiting. ---" && exit 1

    handle_error() {
        local EXITCODE=$?
        local ACTION=$1
        rm -f VERSION VERSION.ini
        printf '%s\n' "--- Failed to $ACTION Skyscraper v${LATEST}, exiting with code $EXITCODE ---"
        popd >/dev/null 2>&1
        exit $EXITCODE
    }

    pushd "$(dirname -- "$(readlink -f -- "$0")")" >/dev/null 2>&1 || exit
    source VERSION.ini 2>/dev/null || VERSION=""
    if [ "$LATEST" != "$VERSION" ]; then
        printf '\n%s\n' "--- Fetching Skyscraper v$LATEST ---"
        tarball="${LATEST}.tar.gz"
        wget -nv https://github.com/Gemba/skyscraper/archive/"$tarball" || handle_error "fetch"

        printf '\n%s\n' "--- Unpacking ---"
        tar_bin='tar'
        [[ "$OSTYPE" == "darwin"* ]] && tar_bin='gtar'
        $tar_bin xzf "$tarball" --strip-components 1 --overwrite || handle_error "unpack"
        rm -f "$tarball"

        if [[ "$1" == "xdg" ]]; then
            if [[ "$OSTYPE" == "darwin"* ]]; then
                sed -i '' "s|#DEFINES+=XDG|DEFINES+=XDG|" skyscraper.pro
            else
                sed -i "s|#DEFINES+=XDG|DEFINES+=XDG|" skyscraper.pro
            fi
        fi

        printf '\n%s\n' "--- Cleaning out old build if one exists ---"
        make --ignore-errors clean 2>/dev/null
        rm -f .qmake.stash
        if which qmake6 >/dev/null; then
            qmake6
        elif which qmake >/dev/null; then
            QT_SELECT=5 qmake
        else
            handle_error "clean old"
        fi

        if [[ "$OSTYPE" == "darwin"* ]]; then
            printf '\n%s\n' "--- macOS : Pre-building adjustment ---"
            sed -i '' "s|CC *= .*|CC             = /usr/bin/gcc|" Makefile
            sed -i '' "s|CXX *= .*|CXX           = /usr/bin/g++|" Makefile
        fi

        printf '\n%s\n' "--- Building Skyscraper v$LATEST ---"
        jobs=$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu)
        make -j "$jobs" || handle_error "build"

        printf '\n%s\n' "--- Installing Skyscraper v$LATEST ---"
        sudo make install || handle_error "install"

        if [[ "$OSTYPE" == "darwin"* ]]; then
            printf '\n%s\n' "--- macOS : extract binary ---"
            mv Skyscraper.app/Contents/MacOS/Skyscraper Skyscraper
            rm -rf Skyscraper.app
        fi

        printf '\n%s\n' "--- Skyscraper has been updated to v$LATEST ---"
    else
        printf '\n%s\n' "--- Skyscraper is already the latest version, exiting ---"
        printf '%s\n' "Hint: You can force a reinstall by removing the VERSION.ini file by"
        printf '%s\n' "running 'rm VERSION.ini'. Then run $0 again."
    fi
    popd >/dev/null 2>&1 || exit
}
