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
	LATEST=$(wget -q -O - "https://api.github.com/repos/Gemba/skyscraper/releases/latest" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')

	if [ ! -f VERSION ]; then
		echo "VERSION=0.0.0" >VERSION
	fi
	source VERSION

	handle_error() {
		local EXITCODE=$?
		local ACTION=$1
		rm -f VERSION VERSION.txt
		echo "--- Failed to $ACTION Skyscraper v${LATEST}, exiting with code $EXITCODE ---"
		exit $EXITCODE
	}

	if [ "$LATEST" != "$VERSION" ]; then
		echo
		echo "--- Fetching Skyscraper v$LATEST ---"
		tarball="${LATEST}.tar.gz"
		wget -nv https://github.com/Gemba/skyscraper/archive/"$tarball" || handle_error "fetch"

		echo
		echo "--- Unpacking ---"
		tar_bin='tar'
		if [[ "$OSTYPE" == "darwin"* ]]; then
			tar_bin='gtar'
		fi
  		$tar_bin xzf "$tarball" --strip-components 1 --overwrite || handle_error "unpack"
		rm -f "$tarball"

		echo
		echo "--- Cleaning out old build if one exists ---"
		make --ignore-errors clean
		rm -f .qmake.stash
		qmake || handle_error "clean old"

		if [[ "$OSTYPE" == "darwin"* ]]; then
			echo
			echo "--- MacOS : Pre-building adjustment ---"
			mv VERSION VERSION.txt
			sed -i '' "s|CC *= .*|CC             = /usr/bin/gcc|" Makefile
			sed -i '' "s|CXX *= .*|CXX           = /usr/bin/g++|" Makefile
		fi

		echo
		echo "--- Building Skyscraper v$LATEST ---"
		make -j$(nproc) || handle_error "build"

		echo
		echo "--- Installing Skyscraper v$LATEST ---"
		sudo make install || handle_error "install"

		if [[ "$OSTYPE" == "darwin"* ]]; then
			echo
			echo "--- MacOS : extract binairy ---"
			mv Skyscraper.app/Contents/MacOS/Skyscraper Skyscraper
			rm -rf Skyscraper.app
			mv VERSION.txt VERSION
		fi

		echo
		echo "--- Skyscraper has been updated to v$LATEST ---"
	else
		echo
		echo "--- Skyscraper is already the latest version, exiting ---"
		echo "Hint: You can force a reinstall by removing the VERSION file by"
		echo "running 'rm VERSION'. Then run ./update_skyscraper.sh again."
	fi
	exit
}
