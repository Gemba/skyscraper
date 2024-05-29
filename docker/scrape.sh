#!/bin/bash

# Usage:
#   ./scrape.sh [ROMS_DIR] [PLATFORM] [SOURCE]
# Example
#   ./scrape.sh ./roms/snes snes screenscraper

TAG=skyscraper

ROMS_DIR=$1
PLATFORM=$2
SOURCE=$3

docker run \
  -v "$ROMS_DIR:/tmp/roms/$PLATFORM" \
  -v "$(pwd)/cache:/tmp/skyscraper_cache" \
  $TAG \
  -p $PLATFORM -s $SOURCE -i /tmp/roms/$PLATFORM -d /tmp/skyscraper_cache
