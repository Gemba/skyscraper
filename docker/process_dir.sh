#!/bin/bash

# Usage:
#   ./process_dir.sh [ROMS_DIR] {source1} {source2} ...
# Example
#   ./process_dir.sh ~/roms
#   ./process_dir.sh ~/roms screenscraper > output.txt
#
# Note: can be use for Emulation Station roms folder, but platform name can be different. ES's genesis is megadrive for skyscraper.
# Defaults to screenscraper and thegamesdb sources if none are specified.

TAG=skyscraper

PLATFORMS=$1/*
shift

SOURCES=("$@")
if [ ${#SOURCES[@]} -eq 0 ]; then
  SOURCES=(screenscraper thegamesdb)
fi

mkdir cache

for file_name in $PLATFORMS; do
  echo $file_name
  if [[ -d "$file_name" ]]; then
    PLATFORM_RAW=${file_name##*/}
    PLATFORM="$(tr [:upper:] [:lower:] <<<"$PLATFORM_RAW")"

    echo "Processing ${PLATFORM}..."
    echo "Mounting $file_name:./roms/$PLATFORM..."

    for source in "${SOURCES[@]}"; do
      echo "Processing $PLATFORM on $source..."

      ./scrape.sh $file_name $PLATFORM $source
    done

    ./save.sh $file_name $PLATFORM
  fi
done
