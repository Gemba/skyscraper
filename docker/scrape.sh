#!/bin/bash

# Usage:
#   ./scrape.sh [ROMS_DIR] [PLATFORM] [SOURCE]
# Example
#   ./scrape.sh ./roms/snes snes screenscraper

TAG=skyscraper

ROMS_DIR=$1
PLATFORM=$2
SOURCE=$3

if [ -f ./config.ini ]; then
  docker run \
    -v "$ROMS_DIR:/tmp/roms/$PLATFORM" \
    -v "$(pwd)/cache:/tmp/skyscraper_cache" \
    -v "$(pwd)/config.ini:/root/.skyscraper/config.ini" \
    $TAG \
    -p $PLATFORM -s $SOURCE -i /tmp/roms/$PLATFORM -d /tmp/skyscraper_cache
else
  docker run \
    -v "$ROMS_DIR:/tmp/roms/$PLATFORM" \
    -v "$(pwd)/cache:/tmp/skyscraper_cache" \
    $TAG \
    -p $PLATFORM -s $SOURCE -i /tmp/roms/$PLATFORM -d /tmp/skyscraper_cache
fi
