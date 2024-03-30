#!/bin/bash

# Usage:
#   ./process_dir.sh [ROMS_DIR]
# Example
#   ./process_dir.sh ~/roms
#   ./process_dir.sh ~/roms > output.txt
#
# Note: can be use for Emulation Station roms folder, but platform name can be different. ES's genesis is megadrive for skyscraper,

TAG=skyscraper

PLATFORMS=$1/*

mkdir cache

for file_name in $PLATFORMS; do
  echo $file_name
  if [[ -d "$file_name" ]]; then
    PLATFORM_RAW=${file_name##*/}
    PLATFORM="$(tr [:upper:] [:lower:] <<<"$PLATFORM_RAW")"

    echo "Processing ${PLATFORM}..."
    echo "Mounting $file_name:./roms/$PLATFORM..."

    for source in "screenscraper" "thegamesdb"; do
      echo "Processing $PLATFORM on $source..."

      ./scrape.sh $file_name $PLATFORM $source
    done

    ./save.sh $file_name $PLATFORM
  fi
done
