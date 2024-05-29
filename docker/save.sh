#!/bin/bash

# Usage:
#   ./scrape.sh [ROMS_DIR] [PLATFORM]
# Example
#   ./scrape.sh ./roms/snes snes

TAG=skyscraper

ROMS_DIR=$1
PLATFORM=$2

docker run \
  -v "$ROMS_DIR:/tmp/roms/$PLATFORM" \
  -v "$(pwd)/cache:/tmp/skyscraper_cache" \
  $TAG \
  -p $PLATFORM -i /tmp/roms/$PLATFORM -d /tmp/skyscraper_cache --flags relative,unattendskip
