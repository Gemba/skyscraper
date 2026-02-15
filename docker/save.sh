#!/bin/bash

# Usage:
#   ./save.sh [ROMS_DIR] [PLATFORM]
# Example
#   ./save.sh ./roms/snes snes

TAG=skyscraper

ROMS_DIR=$1
PLATFORM=$2

docker run \
  -v "$ROMS_DIR:/tmp/roms/$PLATFORM" \
  -v "$(pwd)/cache:/tmp/skyscraper_cache" \
  $TAG \
  -p $PLATFORM -i /tmp/roms/$PLATFORM -g /tmp/roms/$PLATFORM -o /tmp/roms/$PLATFORM/media -d /tmp/skyscraper_cache --flags relative,unattend
