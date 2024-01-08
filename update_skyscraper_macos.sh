#! /usr/bin/env bash
{
	LATEST=$(wget -q -O - "https://api.github.com/repos/Gemba/skyscraper/releases/latest" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')

	if [ ! -f VERSION ]; then
		echo "VERSION=0.0.0" >VERSION
	fi
	source VERSION

	handle_error() {
		local EXITCODE=$?
		local ACTION=$1
		rm --force VERSION
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
		gtar xzf "$tarball" --strip-components 1 --overwrite || handle_error "unpack"
		rm -f "$tarball"

        echo
        echo "--- MacOS : Pre-Cleaning adjustment ---"
        mv VERSION VERSION.txt

		echo
		echo "--- Cleaning out old build if one exists ---"
		make --ignore-errors clean
		rm -f .qmake.stash
		qmake || handle_error "clean old"

        echo
        echo "--- MacOS : Pre-Building adjustment ---"
        sed -i '' "s|CC *= .*|CC            = /usr/bin/gcc|" Makefile
        sed -i '' "s|CXX *= .*|CXX           = /usr/bin/g++|" Makefile
       
		echo
		echo "--- Building Skyscraper v$LATEST ---"
		make -j$(nproc) || handle_error "build"

		echo
		echo "--- Installing Skyscraper v$LATEST ---"
		sudo make install || handle_error "install"

        echo
        echo "--- MacOS : extract binairy ---"
        mv Skyscraper.app/Contents/MacOS/Skyscraper Skyscraper
        rm -rf Skyscraper.app
        mv VERSION.txt VERSION

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
